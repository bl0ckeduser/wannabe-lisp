#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "wannabe-lisp.h"

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
