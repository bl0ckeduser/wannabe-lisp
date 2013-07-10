#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

int interactive;

int main(int argc, char **argv)
{
	char buf[1024 * 1024];
	char lin[1024];
	char *p;
	list_t *expr = new_list();
	env_t *global = new_env();
	int i, c;
	char *ptr, *old;

	/* Set up the built-in procedure symbols 
	 * for arithmetic, comparisons, etc. */
	install_primitives(global);

	/* check for "./lisp -i" invocation -- interactive mode */
	interactive = argc == 2 && !strcmp(argv[1], "-i");

	while (!feof(stdin)) {
		/* read a (syntactic) line */
		do_read(buf);

		/* parse input into a tree */
		build(expr, buf);

		/* eval and print */
		if (interactive) {
			printout(eval(expr, global));
			printf("\n");
		} else {
			eval(expr, global);
		}

		/* clean up for the next iteration */
		sprintf(buf, "");
		expr->cc = 0;
	}

	return 0;
}