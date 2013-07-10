#include <stdio.h>
#include <stdlib.h>
#include "wannabe-lisp.h"

int main()
{
	char buf[1024 * 1024];
	char *p;
	list_t *expr = new_list();
	env_t *global = new_env();

	/* Set up the built-in procedure symbols 
	 * for arithmetic, comparisons, etc. */
	install_primitives(global);

	/* REPL */
	while (1) {
		/* run userfriendly reader (prompt etc) */
		do_read(buf);

		/* parse input into a tree */
		build(expr, buf);

		/* eval and print */
		printout(eval(expr, global));
		printf("\n");

		/* clean up for the next iteration */
		sprintf(buf, "");
		expr->cc = 0;
	}

	return 0;
}
