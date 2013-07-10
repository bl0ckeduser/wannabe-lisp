#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* Based on SICP's metacircular eval/apply tutorial */

list_t* apply(list_t *proc, list_t *args)
{
	env_t *ne;
	int i;

	/* Primitive operation */
	if (proc->cc == 2 && !strcmp(proc->c[0]->head, "PRIM-OP"))
		return do_prim_op(proc->c[1]->head, args);

	/* General case */
	if (proc->type == CLOSURE) {
		/* Check that parameter count matches */
		if (proc->c[0]->cc != args->cc) {
			printf("Error: arg count mismatch in closure application\n");
			goto afail;
		}

		/* Build a new environment */
		ne = new_env();
		ne->father = proc->closure;

		/* Bind the formal parameters 
		 * in the new environment */
		for (i = 0; i < proc->c[0]->cc; ++i)
			env_add(ne, proc->c[0]->c[i]->head,
				REF, args->c[i]);
	
		/* Evaluate the body of the closure
		 * in the new environment which includes
		 * the bound parameters	*/
		return eval(proc->c[1], ne);
	}

afail:
	printf("Error: `apply' has failed\n");
	exit(1);
}
