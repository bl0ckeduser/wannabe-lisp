#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * It's a ring buffer of evaluation step printout strings, to be
 * barfed out upon crashes for purposes of debugging. For example,
 * if the user types "(cadr cadr)", the error
 * "`cdr' expects a linked-list" is given and the following log is
 * made available:
 *
 * ===== DEBUGGING INFO  =====
 * Evaluation trace:
 * (cadr cadr)
 * (car (cdr x))
 * (cdr x)
 * 
 * Symbols: 
 * cadr: #<CLOSURE:0x1046630>
 * x: #<CLOSURE:0x1046630>
 * cdr: (PRIM-OP cdr)
 * ===========================
 *
 * Oh and it's also a symbol table, as the debuglog shows.
 */

/* memory depth (max number of traces kept) */
#define BUFLEN 5

/* symbol storage (max number of symbols kept) */
#define SYMLEN 8

static char **stacktraces;		/* array of trace-strings */
static int ptr;				/* ring buffer index */
static int full;			/* ring buffer full ? */

/* symbol table */
static char **sym_name;
static char **sym_print;
static int sym;
static int sym_ptr;

/* storage for the final complete debug log
 * (it gets stored because it is only printed
 * upon request) */
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

#ifdef DISABLE_STACKTRACER
	return;
#endif
	
	debug_buff_written = 0;
	debug_buff = malloc(ERROR_TEXT_BUFSIZ);
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
		stacktraces[i] = malloc(ERROR_TEXT_BUFSIZ);
		if (!(stacktraces[i]))
			goto fail;
	}

	for (i = 0; i < SYMLEN; ++i) {
		sym_name[i] = malloc(SYMBOL_NAME_MAXLEN);
		if (!(sym_name[i]))
			goto fail;

		sym_print[i] = malloc(LINEBUFSIZ);
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

#ifdef DISABLE_STACKTRACER
	return;
#endif
	
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
#ifdef DISABLE_STACKTRACER
	return;
#endif

	if (strlen(s) > ERROR_TEXT_BUFSIZ)
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

#ifdef DISABLE_STACKTRACER
	return;
#endif

	if (strlen(symb) > SYMBOL_NAME_MAXLEN 
		|| strlen(prnt) > ERROR_TEXT_BUFSIZ)
		return;

	/* replace existing if possible */
	for (i = 0; i < sym; ++i) {
		if (!strcmp(sym_name[i], symb)) {
			strcpy(sym_name[i], symb);
			strcpy(sym_print[i], prnt);
			return;
		}
	}
	
	if (sym_ptr >= SYMLEN)
		sym_ptr = 0;

	strcpy(sym_name[sym_ptr], symb);
	strcpy(sym_print[sym_ptr], prnt);

	if (++sym >= SYMLEN)
		sym = SYMLEN - 1;
	++sym_ptr;
}

void stacktracer_print(char *s)
{
#ifdef DISABLE_STACKTRACER
	return;
#endif

#ifdef JS_GUI
	;
#else
	printf("%s\n", s);
#endif
}

void stacktracer_barf()
{
#ifdef DISABLE_STACKTRACER
	return;
#endif

	if (debug_buff_written)
		stacktracer_print(debug_buff);
	else
		stacktracer_print("no debug log available");
}

void stacktracer_pushbuf(char *s)
{
#ifdef DISABLE_STACKTRACER
	return;
#endif

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

#ifdef DISABLE_STACKTRACER
	return;
#endif

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
	buff = malloc(DEBUGLOG_BUFSIZ);
	if (!buff) {
		error_msg("malloc failed");
		code_error();
	}
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

	stacktracer_reset();
}

