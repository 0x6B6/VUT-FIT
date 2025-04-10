/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h
 * a připravených koster funkcí implementujte binární vyhledávací
 * strom bez použití rekurze.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  /* Valid pointer */
  while (tree) {
    if (key == tree->key) { /* Node has been found */
      *value = &tree->content;
      return true;
    } else if (key > tree->key) { /* Else shift left or right based on key value */
      tree = tree->right;
    } else {
      tree = tree->left;
    }
  }

  return false;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{ /* Valid pointer */
  if (tree) {

    bst_node_t *child = *tree, *parent = NULL, *tmp = NULL;

    while (child) {
      if (key == child->key) { /* Returns on matching key */
        if (child->content.value) { /* Free allocated value and replace it */
          free(child->content.value);
        }
        child->content = value;
        return;
      }

      parent = child; /* Set father node, will be NULL if the searched node is root */

      if (key > child->key) {
        child = child->right;
      } else {
        child = child->left;
      }
    }

    if ((tmp = (bst_node_t*) malloc(sizeof(bst_node_t))) == NULL) {  /*Allocate new node*/
      perror("Resource allocation failure");
      return;
    };

    *tmp = (bst_node_t) {
      .key = key,
       .content = value,
        .left = NULL,
         .right = NULL
    };

    if (*tree == NULL) {  /*Empty (NULL) root*/
      *tree = tmp;
      return;
    }

    if (key > parent->key) { /* New leaf placement depends on its own and its parents key value  */
      parent->right = tmp;
    } else {
      parent->left = tmp;
    }
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  if (tree == NULL || *tree == NULL) /* Invalid pointer(s) */
    return;

  bst_node_t *rightmost = *tree, *parent_rightmost = NULL, *tmp = NULL;

  while (rightmost->right) { /* Get rightmost leaf */
    parent_rightmost = rightmost;
    rightmost = rightmost->right;
  }

  if (rightmost->left) { /* Reconnection of nodes, incase the parent node is NOT a terminal leaf */
    tmp = target->left;
    target->left = rightmost->left;
    target->left->left = tmp;
  }

  if (target->content.value) {
    free(target->content.value);
  }

  target->content = rightmost->content;
  target->key = rightmost->key;

  if (parent_rightmost) /* If rightmost node has a parent, set its right pointer to NULL, since its child is deleted */
    parent_rightmost->right = NULL;
  else *tree = NULL; /* Rightmost node has no parent, will only happen if root is the rightmost */

  free(rightmost);
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
  if (tree == NULL || *tree == NULL)
    return;

  bst_node_t *child = *tree, *parent = NULL, *tmp = NULL;

  while (child) {
    if (key == child->key) { /* Node found */
      if (child->left == NULL && child->right == NULL) { /* Terminal leaf (no children) */
        if (parent) { /* Set parents pointer to NULL depending on which side the child is on */
          if (key > parent->key) {
            parent->right = NULL;
          } else {
            parent->left = NULL;
          }
        } else { /* Root has no parent node, gets set to NULL */
          *tree = NULL;
        }
      } else if (child->left && child->right == NULL) { /* Single child node */
        tmp = child->left;
        if (parent)
          parent->left = tmp;
        else *tree = tmp; /* Set root to left subtree */
      } else if (child->left == NULL && child->right) { /* Single child node */
        tmp = child->right;
        if (parent)
          parent->right = tmp;
        else *tree = tmp; /* Set root to right subtree */
      } else { /* More children node */
        bst_replace_by_rightmost(child, &child->left);
        return;
      }
      /* Free child process */
      if (child->content.value)
        free(child->content.value);

      free (child);

      return;
    }

    parent = child;

    if (key > child->key) {
      child = child->right;
    } else {
      child = child->left;
    }
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  stack_bst_t s;
  stack_bst_init(&s);

  bst_node_t *tmp;

  while (*tree) { /* GetLeftMostBranch */
    stack_bst_push(&s, *tree);
    *tree = (*tree)->left;
  }

  while (!stack_bst_empty(&s)) { /* Deletes leftmost, then right child */
    *tree = stack_bst_top(&s);

    stack_bst_pop(&s);

    if ((*tree)->right) {
      tmp = (*tree)->right;

      while (tmp) { /* GetLeftMostSubBranch */
        stack_bst_push(&s, tmp);
        tmp = tmp->left;
      }
    }

    if ((*tree)->content.value)
      free((*tree)->content.value);

    free(*tree);
  }

  *tree = NULL;
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit, bst_items_t *items)
{
  while(tree) {
    stack_bst_push(to_visit, tree);

    bst_add_node_to_items(tree , items);

    tree = tree->left;
  }
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
  stack_bst_t s;
  stack_bst_init(&s);
  bst_leftmost_preorder(tree, &s, items);

  while(!stack_bst_empty(&s)) {
    tree = stack_bst_top(&s);

    stack_bst_pop(&s);

    bst_leftmost_preorder(tree->right, &s, items);
  }

}

/*
 * Pomocná funkce pro iterativní inorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit)
{
  while(tree) {
    stack_bst_push(to_visit, tree);
    tree = tree->left;
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
  stack_bst_t s;
  stack_bst_init(&s);
  bst_leftmost_inorder(tree, &s);

  while(!stack_bst_empty(&s)) {
    tree = stack_bst_top(&s);
    stack_bst_pop(&s);
    bst_add_node_to_items(tree, items);
    bst_leftmost_inorder(tree->right, &s);
  }
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit, stack_bool_t *first_visit)
{
  while(tree) {
    stack_bst_push(to_visit, tree);
    stack_bool_push(first_visit, true);
    tree = tree->left;
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
  stack_bst_t s;
  stack_bool_t sbool;
  stack_bst_init(&s);
  stack_bool_init(&sbool);
  bst_leftmost_postorder(tree, &s, &sbool);

  while(!stack_bst_empty(&s)) {
    tree = stack_bst_top(&s);

    bool fromLeft = stack_bool_pop(&sbool);

    if (fromLeft) {
      stack_bool_push(&sbool, false);
      bst_leftmost_postorder(tree->right, &s, &sbool);
    } else {
      stack_bst_pop(&s);
      bst_add_node_to_items(tree, items);
    }
  }
}
