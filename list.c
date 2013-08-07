#include <stdio.h>
#include <stdlib.h>
#include "wannabe-lisp.h"

/*
 * This file contains helper routines
 * for the data structure `list_t',
 * which is used for all the runtime
 * objects in this interpreter,
 * including things other than lists
 * like numbers and symbols.
 * 
 * The `type' field shows what kind
 * of object we're dealing with.
 */

/* Create a new list object */
list_t *new_list(void)
{
	list_t* nl = c_malloc(sizeof(list_t));
	nl->type = SYMBOL;	/* default, see others in `wannabe-lisp.h' */
	nl->val = 0;
	nl->ca = 0;
	nl->cc = 0;
	nl->head = c_malloc(32);
	nl->head[0] = 0;
	nl->c = NULL;
	return nl;
}

/* Add a child to a list object */
void add_child(list_t *parent, list_t* child)
{
	list_t **new;
	int i;

	if (++(parent->cc) >= parent->ca) {
		parent->ca += 16;
	
		new = c_malloc(parent->ca * sizeof(list_t *));

		for (i = 0; i < parent->ca - 16; ++i)
			new[i] = parent->c[i];
		
		parent->c = new;
	}

	parent->c[parent->cc - 1] = child;
}

