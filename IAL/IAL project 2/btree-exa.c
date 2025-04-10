/*
 * Použití binárních vyhledávacích stromů.
 *
 * S využitím Vámi implementovaného binárního vyhledávacího stromu (soubory ../iter/btree.c a ../rec/btree.c)
 * implementujte triviální funkci letter_count. Všimněte si, že výstupní strom může být značně degradovaný 
 * (až na úroveň lineárního seznamu). Jako typ hodnoty v uzlu stromu využijte 'INTEGER'.
 * 
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Vypočítání frekvence výskytů znaků ve vstupním řetězci.
 * 
 * Funkce inicilializuje strom a následně zjistí počet výskytů znaků a-z (case insensitive), znaku 
 * mezery ' ', a ostatních znaků (ve stromu reprezentováno znakem podtržítka '_'). Výstup je v 
 * uložen ve stromu.
 * 
 * Například pro vstupní řetězec: "abBccc_ 123 *" bude strom po běhu funkce obsahovat:
 * 
 * key | value
 * 'a'     1
 * 'b'     2
 * 'c'     3
 * ' '     2
 * '_'     5
 * 
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce.
*/
void letter_count(bst_node_t **tree, char *input) {
	bst_init(tree);
	/* Pomocna makra pro prevod na lowercase a filtraci vstupu */
	#define tolower(c) (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c
	#define filter(c) ((c >= 'a' && c <= 'z') || c == ' ') ? c : '_'

	void *p_value;

	while(*input) {
		char c = tolower(*input);
		c = filter(c);

		if((p_value = (int *) malloc(sizeof(int))) == NULL) {
			perror("Resource allocation failure");
			return;
		}

		bst_node_content_t content = (bst_node_content_t) {
			.value = p_value,
			.type = INTEGER
		};

		bst_node_content_t *toBeFound = NULL; /* Hledany obsah */
		*((int *) p_value) = bst_search(*tree, c, &toBeFound) ? *((int*) toBeFound->value) + 1 : 1;

		bst_insert(tree, c, content);
		++input;
	}
}