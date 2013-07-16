#include <stdio.h>
#include <stdlib.h>
#include "wannabe-lisp.h"

list_t *new_list(void)
{
	list_t* nl = c_malloc(sizeof(list_t));
	nl->type = SYMBOL;	/* default, see others in `wannabe-lisp.h' */
	nl->val = 0;
	nl->ca = 0;
	nl->cc = 0;
	nl->head[0] = 0;
	nl->c = NULL;
	return nl;
}

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

