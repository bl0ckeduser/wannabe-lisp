#include <stdio.h>
#include <stdlib.h>
#include "wannabe-lisp.h"

void printout(list_t *l)
{
	extern void printout_iter(list_t* l, int d);
	printout_iter(l, 0);
}

void printout_iter(list_t* l, int d)
{
	int i;

	/* switch based on type */
	if (l->type == NUMBER) {
		printf("%d", l->val);
	} else if(l->type == SYMBOL) {
		printf("%s", l->head);
	} else if (l->type == LIST) { 
		printf("(%s", l->head);
		for (i = 0; i < l->cc; ++i) {
			if (i > 0)
				printf(" ");
			printout_iter(l->c[i], d + 1);
		}
		printf(")");
	} else if (l->type == CLOSURE) {
		printf("#<CLOSURE:%p>", l->closure);
	} else if (l->type == BOOL) {
		if (l->val)
			printf("#t");
		else
			printf("#f");
	}
}


