## Implementační dokumentace k 2. úloze do IPP 2024/2025
Jméno a příjmení: Marek Pazúr

Login: xpazurm00

Cílem projektu je vytvořit skript realizující interpretaci mezikódu jazyka SOL25.

## Návrh a implementace

V rámci skriptu je nutné si vhodně dekomponovat problémy pro zachování jednoduchosti a efektivity při interpretaci.

Skript má k dispozici *sadu* pomocných nástrojů realizujících tyto požadavky.

Mezi ně patří:
- `Parser.php` - třída obsahující podprogram pro rozbor AST v XML formátu
- `Interpret.php` - jádro samotného interpretu
- `Stack.php` - zásobník rámcu pro reprezentaci interní paměti
- `InputReader.php` a `OutputWriter.php` - rozhraní pro vstupně-výstupní operace, poskytnuté rámcem IPP-Core
- `SourceReader.php` - rozhraní pro načtení XML souboru, poskytnuto rámcem IPP-Core

Po načtení XML souboru, který reprezentuje abstraktní syntaktický strom, se provede jeho rozbor.

Výstupem je nový strom vhodný pro interpretaci, reprezentující strukturu programu *SOL25*.

Následně se struktura předá interpretu, který ji začně od počátečního bodu programu interpretovat.

Výstupem skriptu je výstup interpretovaného programu.

### Interní reprezentace programu

Program *SOL25* je interně strukturován jako strom, který má pro všechny konstrukce jazyka připraveny vlastní třídy ve jmenných prostorech `InternalStructs` a `Primitives`.

![Diagram](diagram.svg)

Pro tyto konstrukce je zejména vhodné zmínit třídy `Program` a `ClassType`.

Třída `Program` reprezentuje kořen této struktury a obsahuje tabulku **všech** tříd, tzn. uživatelsky definované i vestavěné.

`ClassType` představuje **uživatelsky** definované třídy, má tabulku **uživatelských** metod `methodTable`, svůj identifikátor `name` sloužící jako *klíč* do tabulky tříd programu a identifikátor své nadtřídy `parent`.
 
Pro všechny **vestavěné** třídy *SOL25* slouží jmenný prostor `Primitives`, který zahrnuje třídy reprezentující tyto *primitivní* typy.

Základním typem těchto vestavěných tříd, ať už v jazyce SOL25 nebo v interpretu, je třída `ObjectClass`, ze které dědí všechny její podtřídy.

`ObjectClass` a její potomci obsahují atribut:
- `name` - sloužící jako *klíč* do tabulky všech dostupných tříd programu
- `parent` - identifikátor nadtřídy
- `methodTable` - tabulka metod **vestavěných** tříd 

Potomci třídy také dědí sadu veřejných statických metod, které si v případě potřeby redefinují. Jinak obsahují své vlastní statické metody sdílející název a funkcionalitu s metodami vestavěných tříd.

Pro instanciaci a práci s oběma typy tříd (uživatelské i vestavěné) je klíčová třída `RuntimeObject`, která slouží jako šablona pro sjednocení instancí tříd.

Mezi její důležité atributy patří:
- `class` - název třídy příslušné instance, sloužící jako klíč do tabulky všech metod
- `attributes` - asociativní pole, které obsahuje instanční atributy, u kterých slouží selektor zprávy jako klíč pro jejich vytvoření či pro přístup k nim. Samotný atribut je opět instancí třídy `RuntimeObject`.
- `primitiveType` - primitivní typ objektu, tzn. název první vestavěné třídy ze které dědí třída instance
- `internal` - interní atribut instance s typem určeným primitivním typem

Také má definovanou sadu veřejných třídních metod pro práci s těmito instancemi:
- `methodLookup` - pro vyhledávání metod v tabulce metod třídy či nadtříd instance
- `getClass` -  vyhledání třídy v tabulce tříd
- `getSuper` - vyhledání nadtřídy v tabulce tříd
- `createAndSetInstance` - vytvoří instanci konkrétní třídy a inicializuje ji danou hodnotou

Pro vyřešení rozsahu platnosti proměnných a parametrů uvnitř jednotlivých bloků slouží zásobník z třídy `Stack`, jenž má definované jednoduché rozhraní (metody typu `pushFrame`, `popFrame`, `topFrame`) pro práci s interním zásobníkem rámců typu `StackFrame`. Tyto rámce pak už obsahují dvě různé asociativní pole (`vars`, `params`) `RuntimeObject` instancí, u kterých je klíčem název parametru či proměnné uvnitř konkrétního bloku. 

### Rozbor XML abstraktního stromu

Rozbor XML stromu provádí metoda `parse()` z třídy `Parser`, která strom prochází kombinací iterativního i rekurzivního průchodu od shora dolů a sestavuje tak hierarchicky interní strukturu programu.

Pro všechny typy uzlů stromu obsahuje třída privátní funkce, jejichž název vždy začíná prefixem `parse` (např. `parseClass`), které pro daný typ uzlu vytvoří instanci třídy z jmenného prostoru `InternalStructs` reprezentující příslušný uzel. Následně jsou atributy vytvořených instancí nastaveny hodnotami atributů XML uzlů stejného typu. Pro extrakci těchto atributů slouží pomocná metoda `getAttribute`.

### Interpretace

Interpretace interní struktury probíhá taktéž rekurzivně i iterativně shora dolů.

Klíčovou části interpretu je metoda `interpretSend`, která obslouží zaslání nějaké zprávy instanci daného objektu.

Metodu příslušející dané zprávě je možno invokovat pouze tehdy, pokud ji třída dané instance obsahuje ve své tabulce metod, nebo zpráva slouží pro definici či přístup k instančnímu atributu. Jinak interpretace skončí s vyjímkou `DNU`.

V případě nalezení metody třídy se invokuje metoda interpretu `invokeMethod`, ve které dojde k rozlišení typu metody.
- Pokud je metoda uživatelsky definovaná, tzn. jde o strukturu stromu, předá se řízení metodě `interpretBlock`, kde dojde k postupnému vyhodnocování příkazů těla bloku.
- Pokud je metoda vestavěná, jedná o typ callable, tzn. odkaz na php metodu dané vestavěné třídy uvnitř jmenného prostoru `Primitives`. Té se předají požadované argumenty a dojde k její invokaci.

Výsledkem všech metod sloužících pro interpretaci je instance `RuntimeObject`.

Počátečním bodem interpretace je metoda `run` třídy `Main`. Interpret tedy zajistí vytvoření instance této třídy a zašle jí bezparametrickou zprávu `run`.

Následně již probíhá postupné zanořování a interpretace daných konstrukcí, tzn. sekvenční interpretace všech příkazů uvnitř bloku, vyhodnocení jejich stromu výrazů, případná invokace metod či bloků zasláním příslušné zprávy, nebo také vytváření samotných instancí tříd.