#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

int interactive;
env_t *global;

void final_clean_up()
{
	gc();
	gc_selfdestroy();
}

int main(int argc, char **argv)
{
	char *buf = malloc(1024 * 1024 * 2);
	char *p;
	list_t *expr = new_list();
	int i, c;
	char *ptr, *old;
	FILE *prefix;

	/* safety herp derp */
	sprintf(buf, "");

	/* Create global environment */
	global = new_env();

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
				call_eval(expr, global);

				/* clean up for the next iteration */
				gc();
				sprintf(buf, "");
				expr = new_list();
			}
		}
		fclose(prefix);
	} else {
		printf("Note: couldn't load `prefix.txt'\n");
	}

	/* check for "./lisp -i" invocation -- interactive mode */
	interactive = argc == 2 && !strcmp(argv[1], "-i");

	while (1) {		
#ifdef GC_STRESS_TEST
		sprintf(buf, "((lambda (x) x) (+ 1 2 3))");
#else
		/* read a (syntactic) line */
		if (!do_read(buf))
			break;
#endif

		if (!*buf)
			break;

		/* parse input into a tree */
		build(expr, buf);

		/* arghhh */
		if (feof(stdin))
			break;

		/* eval and print */
		if (interactive) {
			printout(call_eval(expr, global));
			printf("\n");
		} else {
			call_eval(expr, global);
		}

		/* clean up for the next iteration */
		gc();
		sprintf(buf, "");
		expr = new_list();
	}

	final_clean_up();
	free(buf);

	return 0;
}
