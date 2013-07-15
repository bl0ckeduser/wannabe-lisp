#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

void **ptrs;
char* mark;
int len = 0;
int alloc = 0;

void gc_selfdestroy()
{
	int i = 0;
	for (i = 0; i < len; ++i)
		free(ptrs[i]);
	free(mark);
	free(ptrs);
}

void add_ptr(void *p)
{
	int i;
	for (i = 0; i < len; ++i)
		if (ptrs[i] == p) {
			mark[i] = 0;
			return;
		}
	
	if (++len >= alloc) {
		alloc += 16;
		ptrs = realloc(ptrs, alloc * sizeof(void *));
		mark = realloc(mark, alloc);
		if (!ptrs || !mark) {
			printf("Error: marksweep: realloc has failed\n");
			exit(1);
		}
	}
	mark[len - 1] = 0;
	ptrs[len - 1] = p;
}

void do_mark(void* p, int m)
{
	int i;

	if (!p)
		return;

	for (i = 0; i < len; ++i) {
		if (ptrs[i] == p) {
			mark[i] = m;
			return;
		}
	}

	/* 
	 * add_ptr(p);
	 * do_mark(p, m);
	 */
}

void gc()
{
	int i;
	int orig;

	void **copy_ptr;
	char *copy_mark;

	/* mark */
	marksweep(global);

	/* sweep */
	for (i = 0; i < len; ++i)
		if (ptrs[i] != NULL)
			if (mark[i] == 0)
				free(ptrs[i]);

	/* make a copy of the object list */
	copy_ptr = malloc(len * sizeof(void *));
	if (!copy_ptr) {
		printf("marksweep: malloc failed\n");
		exit(1);
	}
	memcpy(copy_ptr, ptrs, len * sizeof(void *));

	copy_mark = malloc(len);
	memcpy(copy_mark, mark, len);

	/* make a new list with the nonfreed stuff */
	orig = len;
	len = 0;
	for (i = 0; i < orig; ++i) 
		if (copy_mark[i] != 0)
			add_ptr(copy_ptr[i]);

	/* erase temporary copies */
	free(copy_ptr);
	free(copy_mark);
}

void marksweep_list(list_t *l)
{
	int i;

	do_mark(l, 1);
	do_mark(l->c, 1);

	if (l->type == LIST || l->type == CLOSURE || l->type == CONS)
		for (i = 0; i < l->cc; ++i)
			marksweep_list(l->c[i]);

	if (l->type == CLOSURE && l->closure != global)
		marksweep(l->closure);
}

void marksweep(env_t *e)
{
	int i;

	do_mark(e, 1);
	do_mark(e->sym, 1);
	do_mark(e->ty, 1);
	do_mark(e->ptr, 1);

	for (i = 0; i < e->count; ++i)
		do_mark(e->sym[i], 1);

	for (i = 0; i < e->count; ++i)
		marksweep_list(e->ptr[i]);

	if (e->father && e->father != global)
		marksweep(e->father);
}
