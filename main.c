#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

int interactive;

int main(int argc, char **argv)
{
	char *buf = malloc(1024 * 1024 * 2);
	char *p;
	list_t *expr = new_list();
	env_t *global = new_env();
	int i, c;
	char *ptr, *old;
	FILE *prefix;

	/* Set up the built-in procedure symbols 
	 * for arithmetic, comparisons, etc. */
	install_primitives(global);

	/* load up some composite primitive stuff
	 * like caddr */
	if ((prefix = fopen("prefix.txt", "r"))) {
		while ((fgets(buf, 1024, prefix))) {
			strip_nl(buf);
			if (*buf && *buf != ';') {
				build(expr, buf);
				eval(expr, global);
				sprintf(buf, "");
				expr->cc = 0;
			}
		}
		fclose(prefix);
	}

	/* check for "./lisp -i" invocation -- interactive mode */
	interactive = argc == 2 && !strcmp(argv[1], "-i");

	while (1) {		
		/* read a (syntactic) line */
		do_read(buf);

		if (!*buf) {
			break;
		}

		/* parse input into a tree */
		build(expr, buf);

		/* arghhh */
		if (feof(stdin))
			break;

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
