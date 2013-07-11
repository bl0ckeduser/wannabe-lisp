#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

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
		printf("Error: malloc(%ld) has failed\n", size);
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
