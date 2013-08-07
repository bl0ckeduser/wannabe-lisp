#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * It's a ring buffer of evaluation
 * step printout strings, to be barfed
 * out upon crashes for purposes of
 * debugging
 */

/* memory depth */
#define BUFLEN 5

static char **stacktraces;
static int ptr;
static int full;

void stacktracer_reset()
{
	ptr = 0;
	full = 0;
}

void stacktracer_init()
{
	int i;
	stacktraces = malloc(BUFLEN * sizeof(char *));
	if (!stacktraces)
		goto fail;

	for (i = 0; i < BUFLEN; ++i) {
		stacktraces[i] = malloc(1024);
		if (!(stacktraces[i]))
			goto fail;
	}

	stacktracer_reset();

	return;

fail:
	error_msg("stacktracer_init() has failed");
	exit(1);
}

void stacktracer_destroy()
{
	int i;
	
	for (i = 0; i < BUFLEN; ++i) {
		free(stacktraces[i]);
	}

	free(stacktraces);
}

void stacktracer_push(char *s)
{
	if (strlen(s) > 1000)
		return;
	strcpy(stacktraces[ptr], s);
	
	if (++ptr >= BUFLEN) {
		ptr = 0;
		full = 1;
	}
}

/* helper for barf */
void stacktracer_show(char *s)
{
#ifdef JS_GUI
	c_writeback_nl(s);
#else
	printf("%s\n", s);
#endif
}

void stacktracer_barf()
{
	int i;

	stacktracer_show("Debug trace: ");

	if (!full) {
		/*
		 * 0 1 2 3 4
		 * A B C D _
		 *         ^
		 * ptr = 4
		 * full = 0
		 * show 0 to 3
		 */
		for (i = 0; i < ptr; ++i)
			stacktracer_show(stacktraces[i]);
	} else {
		/*
		 * 0 1 2 3 4 
		 * 6 7 8 4 5
		 *       ^
		 * full = 1
		 * ptr = 3
		 * show 3, 4, 0, 1, 2
		 */
		for (i = ptr; i < BUFLEN; ++i)
			stacktracer_show(stacktraces[i]);
		for (i = 0; i < ptr; ++i)
			stacktracer_show(stacktraces[i]);
	}
}

