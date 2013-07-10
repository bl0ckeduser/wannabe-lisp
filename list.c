#include <stdio.h>
#include <stdlib.h>
#include "wannabe-lisp.h"

list_t *new_list(void)
{
	list_t* nl = malloc(sizeof(list_t));
	nl->type = SYMBOL;
	nl->val = 0;
	nl->ca = 0;
	nl->cc = 0;
	nl->head[0] = 0;
	return nl;
}

void add_child(list_t *parent, list_t* child)
{
	if (++(parent->cc) > parent->ca) {
		parent->ca += 16;
		parent->c = realloc(parent->c,
			parent->ca * sizeof(list_t *));
	}

	parent->c[parent->cc - 1] = child;
}

