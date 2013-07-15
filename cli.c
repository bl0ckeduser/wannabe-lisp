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
 * from files
 */
int do_read(char *buf)
{
	return do_read_file(buf, stdin, 0);
}

int do_read_file(char *buf, FILE *f, int silent)
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
		if (interactive && !silent) {
			/* Print prompt */
			if (!i++)
				printf("]=> ");
			else {
				/* new user line, missing parentheses */
				printf("... ");
				/* auto-indent */
				if (bal > 0) {
					for (ind = 0; ind < bal; ++ind) {
						printf("   ");
						if (save_mode)
							fprintf(save_file, "   ");
					}
				}
			}
			fflush(stdout);
		}

		if (!fgets(tmp, 1024, f)) {
			printf("\n");
			if (interactive && !silent) {
				return 0;
			}
			else
				break;
		}

		if (save_mode && *tmp != '\n') {
			fflush(save_file);
			fprintf(save_file, "%s", tmp);
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
						if (save_file)
							fprintf(save_file, ")", tmp);
					}
					if (save_file) {
						fflush(save_file);
						fprintf(save_file, "\n");
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

		/* no use going further on this kind of case */
		if (bal < 0) {
			printf("Error: terrible syntax\n");
			code_error();
		}
	}

	return 1;
}


