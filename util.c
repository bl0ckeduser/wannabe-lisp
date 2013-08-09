#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "wannabe-lisp.h"

/* In this file: misc. helper routines.. */

/*
 * convert back from proper car/cdr 
 * list form to internal representation
 * (1 . (2 . (3 . NIL))) => (1 2 3)
 * 
 * In other words, "linearize" a list
 */
list_t *cons2list(list_t *c)
{
	list_t *curr = c;
	list_t *nw;

	nw = new_list();
	nw->type = LIST;
	if (curr->cc == 2) {
		do {
			add_child(nw, copy_list(curr->c[0]));
			curr = curr->c[1];
		} while (curr->cc == 2);
	}
	return nw;
}

void load_code_from_file(char *fil)
{
	FILE *prefix = NULL;
	char *buf = NULL;
	list_t* expr;

	if ((prefix = fopen(fil, "r"))) {
		/* one kilobyte per logical line is
		 * sufficient except in the
		 * most pathological crazy cases
		 */ 
		buf = malloc(1024);
		if (!buf) {
			error_msg("load: malloc failed");
			code_error();
		}
		while (1) {
			*buf = 0;	
			do_read_file(buf, prefix, 0 /* no REPL CLI */);
			if (!*buf || feof(prefix))
				break;
			if (*buf && !check_comment(buf)) {
				expr = new_list();
				build(expr, buf);
				call_eval(expr, global);

				/* clean up for the next iteration */
				gc();
				sprintf(buf, "");
			}
		}
		fclose(prefix);
		free(buf);
		printf("Loaded `%s'\n", fil);
	} else {
		printf("Note: couldn't load `%s'\n", fil);
		code_error();
	}
}

void error_msg(char *s)
{
#ifdef JS_GUI
	c_writeback("Error: ");
	c_writeback_nl(s);
#else
	printf("Error: %s\n", s);
#endif
}

void fatal_error_msg(char *s)
{
#ifdef JS_GUI
	c_writeback("Fatal error: ");
	c_writeback_nl(s);
#else
	printf("Fatal error: %s\n", s);
#endif
}

/* 
 * Determine whether a line of code
 * is empty or is a comment 
 */
int check_comment(char *s)
{
	while (*s && (*s == ' ' || *s == '\t' || *s == '\n'))
		++s;
	return *s == ';';
}

int code_error()
{
	stacktracer_prebarf();

#ifdef JS_GUI
	exit(0);
#endif

	/*
	 * In interactive mode,
	 * user code errors lead
	 * to a bunch of error messages,
	 * but then the user gets a 
	 * fresh prompt and can carry on.
	 * `repl_jmp' is set in `main.c'
	 */
	if (interactive)
		longjmp(repl_jmp, 1);
	else {
		printf("Shutting down.\n");
		exit(1);
	}
}

list_t *copy_list(list_t *l)
{
	list_t *new = c_malloc(sizeof(list_t));
	memcpy(new, l, sizeof(list_t));
	return new;
}

/* Remove final newline from a string */
void strip_nl(char *s)
{
	while (*s) {
		if (*s == '\n') {
			*s = 0;
			break;
		}
		++s;
	}
}

/* 
 * Do a malloc() and tell the
 * garbage collector about it.
 * If the malloc() fails,
 * printout an error message
 * and halt the program.
 */
void *c_malloc(long size)
{
	void *ptr = malloc(size);
	if (!ptr) {
		fatal_error_msg("malloc() has failed");
		exit(1);
	}
	add_ptr(ptr);
	return ptr;
}

/* 
 * Make a new symbol object having
 * `s' as its name-value
 */
list_t* mksym(char *s)
{
	list_t *sl = new_list();
	strcpy(sl->head, s);
	sl->type = SYMBOL;
	return sl;
}

/* Make a new boolean object ... */
list_t* makebool(int cbool)
{
	list_t *b = new_list();
	b->type = BOOL;
	b->val = cbool;
	return b;
}

int validname(char c)
{
	return
		c != '('
	&&	c != ')'
	&&	c != ' '
	&&	c != '\t'
	&&	c != '\n';
}

int isnum(char c)
{
	return c >= '0' && c <= '9';
}
