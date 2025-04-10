## Implementační dokumentace k 1. úloze do IPP 2024/2025
Jméno a příjmení: Marek Pazúr

Login: xpazurm00

Cílem projektu je vytvořit skript realizující analýzu zdrojového kódu jazyka SOL25.

### Návrh a implementace
Jádrem implementace je užití nástrojů z knihovny `Lark`, která byla zvolena, protože umožňuje generovat syntaktický analyzátor na základě poskytnuté gramatiky v EBNF a efektivně procházet výslednou stromovou strukturu (pomocí nadtříd `Transformer` a `Visitor`).

Gramatika, složená z pravidel a regulárních výrazů, je navržena tak, aby byla minimální, primárně kvůli redukci redundantních uzlů syntaktického stromu. Zároveň zjednodušuje jeho zpracování (např. zachováním uzlů značících konec bloku či příkazu). 

Po načtení zdrojového kódu ze standardního vstupu se provede lexikální a syntaktická analýza, jejímž výstupem je syntaktický strom.

Samotná stromová struktura je složena z uzlů, které reprezentují pravidla a termy gramatiky, včetně jejich vlastností.

Následující fáze se skládá ze dvou etap, kde první z nich je sémantická analýza nad AST, definována ve třídě `Semantics` a pomocných funkcích, zatímco druhá se zaměřuje na transformaci stromu do XML ve třídě `XmlTransformer`.

### Sémantická analýza
Syntaktický strom se prochází shora dolů, aby byly informace dostupné včas, například definice tříd, které mohou být definovány v libovolném pořadí. Každý uzel je analyzován podle svého typu, přičemž může podléhat jedné či více sémantickým kontrolám.

Třída `Semantics` obsahuje instanční metody pro kontroly nad uzly a atributy jako aktuálně zpracovávanou třídu, metodu, identifikátor či tabulku symbolů pro aktuální blok.

V rámci sémantických kontrol je také k dispozici globální tabulka tříd a seznam klíčových slov.

Výše zmíněné tabulky symbolů obsahují kromě samotných identifikátorů také atributy jako příznak inicializace, typ či seznam metod a odkaz na nadřazenou třídu. Datovou strukturou tabulek je asociativní pole.

Pro vyřešení problému zanořování bloků je součástí také zásobník tabulek symbolů a zásobník řetězců identifikátorů.

### Transformace do XML
XML elementy AST jsou generovány pomocí knihovny `xml.etree.ElementTree`.

Strom je tentokrát procházen zdola nahoru, což umožňuje přístup k již transformovaným potomkům aktuálního uzlu, pokud je to potřeba (např. připojení, přeskočení závorek ve výrazech, arity bloku atd.).

Podle typu uzlu je vytvořen XML element s příslušnými atributy. Pokud má potomky, jsou vnořeny dovnitř jeho těla. Výsledný strom se sestavuje postupně od listů až ke kořeni.
