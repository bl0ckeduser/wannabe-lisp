#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "wannabe-lisp.h"

/*
 * convert back from proper car/cdr 
 * list form to internal representation
 * (1 . (2 . (3 . NIL))) => (1 2 3)
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
		buf = malloc(1024 * 1024 * 2);
		if (!buf) {
			error_msg("load: malloc failed");
			code_error();
		}
		while (1) {
			*buf = 0;	
			do_read_file(buf, prefix, 1);
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

int check_comment(char *s)
{
	while (*s && (*s == ' ' || *s == '\t' || *s == '\n'))
		++s;
	return *s == ';';
}

int code_error()
{
	stacktracer_barf();

#ifdef JS_GUI
	exit(0);
#endif

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

list_t* mksym(char *s)
{
	list_t *sl = new_list();
	strcpy(sl->head, s);
	sl->type = SYMBOL;
	return sl;
}

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
