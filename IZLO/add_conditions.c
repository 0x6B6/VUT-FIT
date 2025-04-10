#include <stddef.h>
#include "cnf.h"

//
// LOGIN: <xpazurm00>
//

//Pravidla problemu:
//Musi byt zvolena existujici ulice
//A ZAROVEN
//Musi byt zvolena pouze jedna
//A ZAROVEN
//Ulice musi navazovat Xi[3,4] --> Xi+1[4,?]
//A ZAROVEN
//Nesmi byt znovu pouzita ulice

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 1)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
// Pole streets ma velikost num_of_streets a obsahuje vsechny existujuci ulice
//    - pro 0 <= i < num_of_streets predstavuje streets[i] jednu existujici
//      ulici od krizovatky streets[i].crossroad_from ke krizovatce streets[i].crossroad_to
void at_least_one_valid_street_for_each_step(CNF *formula, unsigned num_of_crossroads, unsigned num_of_streets,
                                             const Street *streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);
    assert(streets != NULL);

    for (unsigned i = 0; i < num_of_streets; ++i) {
        //Pro kazdy jeden krok i, (i == krok).

        Clause *cl = create_new_clause(formula);
        //Vytvoreni nove klauzule v kazdem kroku.

        for (unsigned idx = 0; idx < num_of_streets; ++idx) {
            //Postupne pridavani ulic do klauzule, (idx == index pro pole ulic).

            add_literal_to_clause(cl, true, i, streets[idx].crossroad_from, streets[idx].crossroad_to);
            //Klauzule ve tvaru: (Xi[z_1,k_1] V Xi[z_1,k_2] V Xi[z_1,k_2] ...)
            //V kazdem kroku, musi byt zvolena ALESPON jedna validni ulice z permutaci ulic. Nemuze byt zvolena ulice, ktera neexistuje!
            //napriklad: (Xi[1,2]V Xi[2,3] V Xi[3,4] V Xi[4,5]) ^... (kdyby zde zadna ulice neplatila, tak 0)
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 2)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void at_most_one_street_for_each_step(CNF *formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);

    //MAXIMALNE jedna ulice pro kazdy jeden krok, tzn. ze nelze prochazet >1 ulic zaroven.

    // logika:
    //A = jdu po ulici A; B = jdu po ulici B
    //(A ^ B) = jdu po ulici A a B
    //~(A ^ B) <-> (~A V ~B) Nejdu po ulici A nebo Nejdu po ulici B
    //NOR funkce, plati v pouze v pripade zvoleni jen jedne ulice na stejnem kroku!

    for (unsigned i = 0; i < num_of_streets; ++i) {
        //Pro kazdy stejny krok i, (i == krok)

        for (unsigned z_1 = 0; z_1 < num_of_crossroads; ++z_1) {
            for (unsigned k_1 = 0; k_1 < num_of_crossroads; ++k_1) {
                //Souradnice prvni ulice [z_1,k_1], (z_1 = zacatek 1. ulice, k_1 = konec 1. ulice)

                for (unsigned z_2 = 0; z_2 < num_of_crossroads; ++z_2) {
                    for (unsigned k_2 = 0; k_2 < num_of_crossroads; ++k_2) {
                        //Souradnice druhe ulice [z_2, k_2], (z_2 = zacatek 2. ulice, k_2 = konec 2. ulice)

                        if (z_1 != z_2 || k_1 != k_2) {
                            //Ulice se musi lisit (jejich souradnice), jinak by neplatila podminka v pripade, ze by se pridala stejna ulice.
                            //napriklad: (~Xi(0,0) V ~Xi(0,0) == 0! Protoze: 0 or 0 = 0

                            Clause *cl = create_new_clause(formula);
                            //Vytvoreni klauzule pro kazdou kombinaci ulice.

                            add_literal_to_clause(cl, false, i, z_1, k_1);
                            add_literal_to_clause(cl, false, i, z_2, k_2);
                            //Klauzule ve tvaru: (~A V ~B) <==> ~(A ^ B)
                            //Napriklad: (~Ai=0[z_1,k_1] V ~Bi=0[z_2,k_2]) ^ (~Ai=1[z_1,k_1] V ~Bi=1[z_2,k_2]) ...
                        }
                    }
                }
            }
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 3)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_connected(CNF *formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);

    //logika:
    //Ulice v ramci cesty na sebe navazuji, pokud ulice [z,k] je i-tym krokem cesty, kde i je z <0, U-1),s
    //potom ulice v kroce i+1 zacina v k.


    for (unsigned i = 0; i < num_of_streets - 1; ++i) {
        // pro kazdy krok i, chceme stejny krok, num of streets - 1

        for (unsigned z = 0; z < num_of_crossroads; ++z) {
            for (unsigned k_1 = 0; k_1 < num_of_crossroads; ++k_1) {
                //Souradnice prvni ulice, (z = zacatek 1. ulice, k_1 = konec 1. ulice)

                Clause *cl = create_new_clause(formula);
                //Nova klauzule se vytvori az pro kazdou novou ulici 1, kde chceme kontrolovat, zda na ni dalsi ulice navazuji

                for (unsigned k_2 = 0; k_2 < num_of_crossroads; ++k_2) {
                    //Souradnice druhe ulice, (k_1 je zde zacatkem ulice, k_2 je konec 2. ulice)

                    add_literal_to_clause(cl, false, i, z, k_1);
                    add_literal_to_clause(cl, true, i + 1, k_1, k_2);
                    //Klauzule ve tvaru: ~A V B  <==> A --> B (implikace: pokud ulice, potom ulice...)
                    //(~Ai[x,y] V Bi[y,z]...) ^
                    //Napriklad: (~Ai[0,1] V Bi[1,2] V Ci[1,3] V Di[1,4]...) ^
                }
            }
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 4)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_do_not_repeat(CNF *formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);

    for (unsigned i = 0; i < num_of_streets; ++i) {
        // pro kazdy krok i
        for (unsigned j = 0; j < num_of_streets; ++j) {
            //krok j
            if (i != j) {
                // kroky se nesmi rovnat
                for (unsigned z = 0; z < num_of_crossroads; ++z) {
                    for (unsigned k = 0; k < num_of_crossroads; ++k) {
                        // pro kazdu dvojici krizovatek (z, k)
                        // Obdobny pripad jako v at_most_one_street_for_each_step, tentokrat misto jedne ulice
                        // muze byt zvolena ulice jen jednou za celou formuli.
                        Clause *cl = create_new_clause(formula);
                        add_literal_to_clause(cl, false, i, z, k);
                        add_literal_to_clause(cl, false, j, z, k);
                    }
                }
            }
        }
    }
}