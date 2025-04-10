/*
 * Tabulka s rozptýlenými položkami
 *
 * S využitím datových typů ze souboru hashtable.h a připravených koster
 * funkcí implementujte tabulku s rozptýlenými položkami s explicitně
 * zretězenými synonymy.
 *
 * Při implementaci uvažujte velikost tabulky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptylovací funkce která přidělí zadanému klíči index z intervalu
 * <0,HT_SIZE-1>. Ideální rozptylovací funkce by měla rozprostírat klíče
 * rovnoměrně po všech indexech. Zamyslete sa nad kvalitou zvolené funkce.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializace tabulky — zavolá sa před prvním použitím tabulky.
 */
void ht_init(ht_table_t *table) {
  for (int i = 0;  i < HT_SIZE; ++i) /* Set every item to NULL */
    (*table)[i] = NULL;
}

/*
 * Vyhledání prvku v tabulce.
 *
 * V případě úspěchu vrací ukazatel na nalezený prvek; v opačném případě vrací
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {

  int mapped_key = get_hash(key); // Map key to index

  ht_item_t *item = (*table)[mapped_key];

  while (item) { /* If item exists, traverse through list */
    if (!strcmp(item->key, key))
      return item;

    item = item->next;
  }

  return NULL;
}

/*
 * Vložení nového prvku do tabulky.
 *
 * Pokud prvek s daným klíčem už v tabulce existuje, nahraďte jeho hodnotu.
 *
 * Při implementaci využijte funkci ht_search. Pri vkládání prvku do seznamu
 * synonym zvolte nejefektivnější možnost a vložte prvek na začátek seznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {
  int mapped_key = get_hash(key); // Map key to index

  ht_item_t *item = ht_search(table, key), *new = NULL;

  if (item) { /* item exists */
    item->value = value;
  } else { /* item is not in table */

    item = (*table)[mapped_key]; /* LIST or NULL */

    if ((new = (ht_item_t*) malloc(sizeof(ht_item_t))) == NULL) { /* Allocate new item */
      /* Error */
      return ;
    }

    *new = (ht_item_t) {
      .key = key,
      .value = value,
      .next = item /* either null or previous item list */ 
    };

    (*table)[mapped_key] = new;
  }
}

/*
 * Získání hodnoty z tabulky.
 *
 * V případě úspěchu vrací funkce ukazatel na hodnotu prvku, v opačném
 * případě hodnotu NULL.
 *
 * Při implementaci využijte funkci ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {

  ht_item_t *item = ht_search(table, key);

  if (item) {
    return &item->value;
  }

  return NULL;
}

/*
 * Smazání prvku z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje přiřazené k danému prvku.
 * Pokud prvek neexistuje, funkce nedělá nic.
 *
 * Při implementaci NEPOUŽÍVEJTE funkci ht_search.
 */
void ht_delete(ht_table_t *table, char *key) {
  int mapped_key = get_hash(key); // Map key to index

  ht_item_t *item = (*table)[mapped_key], *prev = NULL;

  if (item) {
    while(strcmp(item->key, key)) { /* While matching key found */
      prev = item;          /* Backtrack previous item */
      item = item->next;
    }

    if(prev) /* Previous item exists, item is in the middle */
      prev->next = item->next; /* NULL or rest of the list */
    else  /* Previous doesnt exist, item is first in the list */
      (*table)[mapped_key] = item->next;

    free(item);
  }
}

/*
 * Smazání všech prvků z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje a uvede tabulku do stavu po
 * inicializaci.
 */
void ht_delete_all(ht_table_t *table) {
  ht_item_t *item = NULL, *tmp = NULL;

  for (int i = 0; i < HT_SIZE; ++i) /* For each list */
  {
    item = (*table)[i];

    while (item) { /* For each item in list */
      tmp = item->next;
      
      free(item);

      item = tmp; /* Get next item */
    }

    (*table)[i] = NULL;
  }
}
