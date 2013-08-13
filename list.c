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
	
	/* val: integer storage for numbers and bools */
	nl->val = 0;

	/* head: string used for symbol names
	 * (initialized to "") */
	nl->head = c_malloc(SYMBOL_NAME_MAXLEN);
	nl->head[0] = 0;
	
	/* c: children array
	 * ca: children array allocation count
	 * cc: children array member count */
	nl->c = NULL;
	nl->ca = 0;	
	nl->cc = 0;
	
	return nl;
}

/* Add a child to a list object */
void add_child(list_t *parent, list_t* child)
{
	list_t **new;
	int i;

	/* expand children array if necessary */
	if (++(parent->cc) >= parent->ca) {
		parent->ca += ALLOC_EXPAND;
	
		/* realloc() is not used because i cannot figure
		 * out how to make it work with the gc */
		new = c_malloc(parent->ca * sizeof(list_t *));

		for (i = 0; i < parent->ca - ALLOC_EXPAND; ++i)
			new[i] = parent->c[i];
		
		parent->c = new;
	}

	parent->c[parent->cc - 1] = child;
}

