/*
 *   Autor: Marek Pazur - xpazurm00
 *   IZP Projekt 1
 *   Prace s textem
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Konstanta udavajici maximalni velikost radku - 100 + 2 znaku.
 * 100 znaku pro samotne slovo, vcetne 2 znaku pro '\n' a '\0'
 */
#define MAX_LINE_SIZE 102
/*Konstanta udavajici velikost ASCII tabulky*/
#define ASCII_SIZE 127
/*Makro pro kontrolu, zda je buffer prazdny*/
#define is_empty(a) (a == 0) ? 1 : 0

/*
 * struct: filter
 *
 * keys: ukladani povolenych znaku
 * first_match:  ulozeni prvni shody
 * matches: pocet shod
 * */
struct filter {
    char keys[ASCII_SIZE];
    char first_match[MAX_LINE_SIZE];
    size_t matches;
};

/*
 * Funkce: init_input
 * ------------------
 * Inicializuje strukturu filter.
 *
 * *filter: ukazatel na strukturu filter
 */
void init_filter(struct filter *filter);

/*
 * Funkce: process_input
 * --------------------
 * Funkce pro nacitani a zpracovani vstupu z STDIN.
 *
 * Nacita retezce a zaroven je filtruje podle shody s prefixem dokud nedojde k EOF.
 * V pripade shody inkrementuje pocet shod, ulozi prvni shodu a dalsi mozne klavesy do struktury filter.
 *
 * Retezec muze byt dlouhy maximalne 100 znaku.
 * V pripade, ze bude retezec delsi, funkce ignoruje zbytek dokud nenarazi na konec radku nebo na EOF.
 *
 * *filter: ukazatel na strukturu filter
 * *prefix: ukazatel na retezec prefixu
 *
 * Vraci: EXIT_SUCCESS pri uspesnem dokonceni funkce
 *        EXIT_FAILURE pri neuspesnem cteni dat z STDIN
 */
int process_input(struct filter *filter, char *prefix);

/*
 * Funkce: to_uppercase
 * --------------------
 * Pokud je znak male pismeno, tak ho funkce prevede na velke pismeno.
 *
 * key: znak urcen k prevodu
 *
 * Vraci: Velke pismeno
 *        Neupraveny znak, v pripade ze se nejedna o male pismeno
 */
char to_uppercase(char key);

/*
 * Funkce: has_prefix
 * ------------------
 * Zjisti zda retezec z STDIN obsahuje uzivatelem zadany prefix.
 *
 * *string: ukazatel na retezec z STDIN
 * *prefix: ukazatel na retezec prefixu
 *
 * Vraci: EXIT_SUCCESS v pripade shody
 *        EXIT_FAILURE v pripade neshody
 */
int has_prefix(const char *string, const char *prefix);

/*
 * Funkce: key_to_index
 * --------------------
 * Najde dalsi moznou klavesu k povoleni.
 *
 * *string: Uzivatelem zadany retezec
 * *prefix: Uzivatelem zadany
 *
 * Vraci: Moznou klavesu k povoleni
 */
int key_to_index(char *string, char *prefix);

/*Funkce: string_to_upper
 *
 * Pomocna funkce pro prevod malych pismen v retezci na velke.
 *
 * *string: ukazatel na retezec
 */
void string_to_upper(char *string);


int main(int argc, char **argv) {
    char *prefix;
    struct filter filter;

    /*Osetreni argumentu programu*/
    if (argc == 1) {
        prefix = "";
    } else if (argc == 2) {
        prefix = argv[1];
    } else {
        fprintf(stderr, "error: invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    init_filter(&filter);

    /*Zpracovani vstupu*/
    if (process_input(&filter, prefix) == EXIT_FAILURE) {
        fprintf(stderr, "error: reading from database failed\n");
        return EXIT_FAILURE;
    }

    /*Vypis programu podle poctu shod*/
    switch (filter.matches) {
        case 0:
            printf("Not found\n");
            break;
        case 1:
            /*Program vytiskne jedinou shodu*/
            printf("Found: ");
            for (int i = 0; filter.first_match[i] != '\0'; ++i)
                putchar(to_uppercase(filter.first_match[i]));
            putchar('\n');
            break;
        default:
            /*Program vytiskne pouze tisknutelne znaky ASCII*/
            printf("Enable: ");
            for (int i = ' '; i < ASCII_SIZE; ++i)
                if (filter.keys[i] != 0) {
                    putchar(to_uppercase((char) i));
                }
            putchar('\n');
            break;
    }

    return EXIT_SUCCESS;
}

void init_filter(struct filter *filter) {
    memset(filter->keys, 0, ASCII_SIZE);
    memset(filter->first_match, 0, MAX_LINE_SIZE);
    filter->matches = 0;
}

int process_input(struct filter *filter, char *prefix) {
    /* Vstupni buffer */
    char buffer[MAX_LINE_SIZE] = {0};

    /*Nacitani z STDIN dokud funkce nedojde k EOF*/
    while (fgets(buffer, MAX_LINE_SIZE, stdin) != NULL) {

        /* V pripade, ze je retezec delsi jak 100 znaku, ignoruje zbytek do konce radku nebo EOF.
         *
         * Zjisti se podle predposledniho znaku v bufferu - pokud funkce fgets() nacte mene nez MAX_LINE_SIZE - 1 znaku,
         *  zahrnuje '\n' do nacteneho retezce a zajisti tim nacteni celeho radku - predposledni znak musi byt '\n' nebo '\0'.
         *
         * V pripade ze precte MAX_LINE_SIZE - 1 znaku, znamena to ze radek prekrocil maximalni povolenou velikost
         * 100 znaku a na predposlednim znaku nebude '\n' ani '\0', tudiz bude zbytek radku ignorovan.
         * */
        if (buffer[MAX_LINE_SIZE - 2] != '\0' && buffer[MAX_LINE_SIZE - 2] != '\n') {
            char c = buffer[100];

            while (c != '\n' && c != EOF)
                c = (char) getchar();
        }

        /*Nahrazeni predposledniho znaku retezce za '\0', kvuli funkci fgets()*/
        buffer[strlen(buffer) - 1] = '\0';

        /*Kontrola zda retezec obsahuje prefix*/
        if (has_prefix(buffer, prefix) == EXIT_SUCCESS) {

            string_to_upper(buffer);

            /*Osetreni duplikatu - pokud se v databazi objevi vicekrat stejne slovo, ktere se primo shoduje s prefixem,
             * tak se filter->matches nebude inkrementovat.*/
            if (strcmp(buffer, filter->first_match) != 0) {
                ++filter->matches;
            }

            /*Ulozeni prvni shody*/
            if (is_empty(filter->first_match[0])) {
                strcpy(filter->first_match, buffer);
            }

            /*Ulozeni povolenych klaves*/
            filter->keys[key_to_index(buffer, prefix)] = 1;
        }

    }

    /*Kontrola zda je buffer prazdny, pokud ano, tak se vypise chybove hlaseni a funkce se ukonci.*/
    if (is_empty(buffer[0])) {
        fprintf(stderr, "error: database file is empty\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char to_uppercase(char key) {
    /*Pokud je znak male pismeno, odecte se od nej hodnota 'a' - 'A' (32), timto krokem se prevede na velke pismeno*/
    if (key >= 'a' && key <= 'z') {
        key -= ('a' - 'A');
        return key;
    }

    return key;
}

int has_prefix(const char *string, const char *prefix) {
    /*Kontrola, zda retezec obsahuje zadany prefix porovnavanim jednotlivych znaku, dokud prefix nedojde k
     * ukoncovacimu znaku '\0'*/
    for (int i = 0; prefix[i] != '\0'; ++i) {
        if (to_uppercase(string[i]) != to_uppercase(prefix[i]))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int key_to_index(char *string, char *prefix) {
    /*Nalezeni dalsi mozne klavesy pouzitim delky prefixu jako indexu .
     * Pokud je klavesa male pismeno, prevede se nejdriv na velke pomoci funkce to_uppercase().*/
    if (string[strlen(prefix)] >= 'a' && string[strlen(prefix)] <= 'z') {
        return (int) to_uppercase(string[strlen(prefix)]);
    }
    return (int) string[strlen(prefix)];
}

void string_to_upper(char *string) {
    /*Iterace retezce a prevod malych pismen na velke*/
    for (int i = 0; string[i] != '\0'; ++i) {
        string[i] = to_uppercase(string[i]);
    }
}