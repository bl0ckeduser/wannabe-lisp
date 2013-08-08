#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * Old-school primitive naive basic unsophisticated
 * mark-sweep garbage collector, coded based on the
 * example and explanations in the SICP video series.
 */

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
	/* prevent adding things twice */
/*
	for (i = 0; i < len; ++i)
		if (ptrs[i] == p) {
			mark[i] = 0;
			return;
		}
*/	
	
	/* expand list if needed */
	if (++len >= alloc) {
		alloc += 16;
		ptrs = realloc(ptrs, alloc * sizeof(void *));
		mark = realloc(mark, alloc);
		if (!ptrs || !mark) {
			fatal_error_msg("marksweep: realloc has failed");
			exit(1);
		}
	}

	/* add pointer to list and set initial zero-mark */
	mark[len - 1] = 0;
	ptrs[len - 1] = p;
}

void do_mark(void* p, int m)
{
	int i;

	if (!p)
		return;

	/* Look for the pointer in the
	 * list and mark it */
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
		fatal_error_msg("marksweep: malloc failed");
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

/*
 * Mark the struct fields of a list object,
 * (as well as the object itself),
 * and recurse onto its list and environment
 * (if it's a closure) children.
 */
void marksweep_list(list_t *l)
{
	int i;

	do_mark(l, 1);
	do_mark(l->c, 1);
	do_mark(l->head, 1);

	if (l->type == LIST || l->type == CLOSURE || l->type == CONS)
		for (i = 0; i < l->cc; ++i)
			marksweep_list(l->c[i]);

	if (l->type == CLOSURE && l->closure != global)
		marksweep(l->closure);
}

/*
 * Mark an environment object as well as 
 * as its struct-fields. Recurse onto its list
 * and environment relations, if any.
 */
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
