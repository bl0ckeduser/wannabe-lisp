#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* 
 * Fairly `user-friendly' CLI reader
 * (gives new lines to add missing parentheses,
 * autocompletes parentheses if desired, etc)
 *  -- Quick & dirty ugly code, however.
 *
 * Note that this is also required for reading
 * from files, and at the moment only stdin
 * is supported.
 */
void do_read(char *buf)
{
	int bal = -1;
	char tmp[1024];
	char *p;
	int i = 0;
	int bl = 0;
	int ind;

	/* Loop as long as there 
	 * are unclosed parentheses */
	while (bal) {
		if (interactive) {
			/* Print prompt */
			if (!i++)
				printf("]=> ");
			else {
				/* new user line, missing parentheses */
				printf("... ");
				/* auto-indent */
				if (bal > 0 && bal < 5) {
					for (ind = 0; ind < bal; ++ind)
						printf("   ");
				}
			}
			fflush(stdout);
		}

		if (!fgets(tmp, 1024, stdin)) {
			printf("\n");
			if (interactive)
				exit(1);
			else
				break;
		}

		if (*tmp == ';')
			continue;

		/* skip empty lines, parser hates them */
		if (*tmp == '\n') {
			if (interactive) {
				if (i == 1) --i;
				/* 
				 * blank line
				 * => autocomplete parentheses
				 *
				 */
				if (++bl > 0 && bal > 0) {
					printf ("... ");
					for (ind = 0; ind < bal; ++ind) {
						printf(")");
						strcat(buf, ")");
					}
					fflush(stdout);
					printf("\n");				
					break;
				}
			}
			continue;
		} else
			bl = 0;

		/* eat newline */
		for (p = tmp; *p; ++p) {
			if (*p == '\n' || *p == EOF) {
				*p = 0;
				break;
			}
		}

		/* add inputted line to complete buffer */
		strcat(buf, tmp);
		strcat(buf, " ");

		/* count parentheses balance */
		bal = 0;
		for (p = buf; *p; ++p) {
			if (*p == '(')
				++bal;
			else if (*p == ')')
				--bal;
		}
	}
}


