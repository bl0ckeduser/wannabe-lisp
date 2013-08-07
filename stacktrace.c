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

/* symbol storage */
#define SYMLEN 8

static char **stacktraces;
static int ptr;
static int full;

static char **sym_name;
static char **sym_print;
static int sym;
static int sym_ptr;

char *debug_buff;
int debug_buff_written;

void stacktracer_reset()
{
	ptr = 0;
	full = 0;
	sym = 0;
	sym_ptr = 0;
}

void stacktracer_init()
{
	int i;
	
	debug_buff_written = 0;

	debug_buff = malloc(1024 * 1024 * 8);
	if (!debug_buff)
		goto fail;
	*debug_buff = 0;

	stacktraces = malloc(BUFLEN * sizeof(char *));
	if (!stacktraces)
		goto fail;

	sym_name = malloc(SYMLEN * sizeof(char *));
	if (!sym_name)
		goto fail;

	sym_print = malloc(SYMLEN * sizeof(char *));
	if (!sym_print)
		goto fail;


	for (i = 0; i < BUFLEN; ++i) {
		stacktraces[i] = malloc(1024);
		if (!(stacktraces[i]))
			goto fail;
	}

	for (i = 0; i < SYMLEN; ++i) {
		sym_name[i] = malloc(1024);
		if (!(sym_name[i]))
			goto fail;

		sym_print[i] = malloc(1024);
		if (!(sym_print[i]))
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

	for (i = 0; i < SYMLEN; ++i) {
		free(sym_name[i]);
		free(sym_print[i]);
	}

	free(stacktraces);
	free(sym_name);
	free(sym_print);
	free(debug_buff);
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

void stacktracer_push_sym(char *symb, char *prnt)
{
	int i;

	/* replace existing */
	for (i = 0; i < SYMLEN; ++i) {
		if (!strcmp(sym_name[i], symb)) {
			strcpy(sym_name[i], symb);
			strcpy(sym_print[i], prnt);
			return;
		}
	}
	
	if (sym_ptr >= SYMLEN)
		sym_ptr = 0;

	if (strlen(symb) > 1000 || strlen(prnt) > 1000)
		return;

	strcpy(sym_name[sym_ptr], symb);
	strcpy(sym_print[sym_ptr], prnt);

	if (++sym >= SYMLEN)
		sym = SYMLEN - 1;
	++sym_ptr;
}

void stacktracer_print(char *s)
{
#ifdef JS_GUI
	;
#else
	printf("%s\n", s);
#endif
}

void stacktracer_barf()
{
	if (debug_buff_written)
		stacktracer_print(debug_buff);
	else
		stacktracer_print("no debug log available");
}

void stacktracer_pushbuf(char *s)
{
#ifdef JS_GUI
	extern void c_writedebug(char *str);
	c_writedebug(s);
	c_writedebug("\\n");
#else
	strcat(debug_buff, s);
	strcat(debug_buff, "\n");
#endif
}

void stacktracer_prebarf()
{
	int i;
	char *buff;

	*debug_buff = 0;
	debug_buff_written = 1;

	stacktracer_pushbuf("===== DEBUGGING INFO  =====");
	stacktracer_pushbuf("Evaluation trace:");

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
			stacktracer_pushbuf(stacktraces[i]);
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
			stacktracer_pushbuf(stacktraces[i]);
		for (i = 0; i < ptr; ++i)
			stacktracer_pushbuf(stacktraces[i]);
	}

	stacktracer_pushbuf("");
	stacktracer_pushbuf("Symbols: ");
	buff = malloc(4096);
	for (i = 0; i < sym; ++i) {
		sprintf(buff, "%s: %s", sym_name[i], sym_print[i]);
		stacktracer_pushbuf(buff);
	}
	stacktracer_pushbuf("===========================");
	stacktracer_pushbuf("  ");
	free(buff);

#ifdef JS_GUI
	extern void c_writeback_nl(char *);
	c_writeback_nl("A debug log has been added to the debug log window.");
#else
	stacktracer_print("A debug log has been prepared. Type (debuglog) to see it.");
#endif
}

