#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* 
 * The routine `do_read_file(buf, f, silent)'
 * reads one `logical line' from the file 
 * handle `f' and writes it to the buffer `buf'.
 * 
 * A `logical line' is a group of code where
 * all parentheses haved been closed; it is
 * often composed of several standard
 * newline-terminated input lines.
 *
 * For example, in the following session:
 *	]=> (define (display-nl x)
 *	...    (display x)
 *	...    (newline))
 * the user has typed three input lines that the
 * reader converts to a single logical line:
 * (define (display-nl x) (display x) (newline))
 *
 * If `silent' is set to 0, the reader acts
 * in "friendly CLI" mode (Python-style),
 * printing prompts, performing auto-indentation,
 * etc.
 */

/* do_read() uses stdin as the file... */
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
		/* Handle prompts/autoindent in interactive mode */
		if (interactive && !silent) {
			/* If i = 0, print a new logical-line prompt */
			if (!i++)
				printf("]=> ");
			else {
				/* The logical-line is incomplete; print
				 * a "..." prompt then auto-indent */
				printf("... ");
				if (bal > 0) {
					for (ind = 0; ind < bal; ++ind) {
						printf("   ");
						/* 
						 * Transcribe the auto-indentation
						 * to the logfile if logging is enabled
						 */
						if (save_mode)
							fprintf(save_file, "   ");
					}
				}
			}
			/* 
			 * Flush out the prompt/autoindent to make sure
			 * it appears -- it's not newline-terminated,
			 * because the rest of the line is the user's code
			 */
			fflush(stdout);
		}

		/*
		 * Read in a line of input. exit the routine
		 * in various ways if this fails.
		 */
		if (!fgets(tmp, 1024, f)) {
			printf("\n");
			if (interactive && !silent) {
				return 0;
			}
			else
				break;
		}

		/* 
		 * Transcribe the line input to the logfile,
		 * if logging and interactive (repl) mode 
		 * are enabled 
		 */
		if (save_mode && !silent && *tmp != '\n') {
			fflush(save_file);
			fprintf(save_file, "%s", tmp);
		}

		/* Check if the line is a comment */
		if (check_comment(tmp)) {
			/* 
			 * If a comment was written on a "]=>" prompt,
			 * write another "]=>" prompt afterwards, not
			 * a "..." 
			 */
			if (i == 1)
				i = 0;
			/* 
			 * Go on to the next line of input; 
			 * do not add the comment to the buffer 
			 */
			continue;
		}

		/* Skip empty lines, parser hates them */
		if (*tmp == '\n') {
			if (interactive && !silent) {
				/*
				 * An empty line given on a "]=>" prompt
				 * leads to a new "]=>" prompt afterwards,
				 * not a "..." prompt.
				 */
				if (i == 1)
					--i;

				/* 
				 * In interactive (repl) mode, blank
				 * lines cause the CLI reader to autocomplete
			 	 * missing closing parentheses. This loop
				 * is ugly because it has to transcribe these
				 * parentheses to the buffer, to stdout, and
				 * also to the logfile if logging is enabled.
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

		/* Trim the newline from the input line */
		for (p = tmp; *p; ++p) {
			if (*p == '\n' || *p == EOF) {
				*p = 0;
				break;
			}
		}

		/* Add the input line to the logical-line buffer */
		strcat(buf, tmp);
		strcat(buf, " ");

		/* Check parentheses balance */
		bal = 0;
		for (p = buf; *p; ++p) {
			if (*p == '(')
				++bal;
			else if (*p == ')')
				--bal;
		}

		/* Abort on negative parenthesis-nest */
		if (bal < 0) {
			error_msg("terrible syntax");
			code_error();
		}
	}

	return 1;
}


