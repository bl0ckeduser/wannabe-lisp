#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * Old-school primitive naive basic unsophisticated
 * mark-sweep garbage collector, coded based on the
 * example and explanations in the SICP video series.
 */
 
/* 
 * A more sophisticated approach, perhaps
 * involving marking and unmarking things at runtime
 * and calling gc() more frequently, will be required
 * to avoid leaks such as in:
 *
 *	(define (loop n)
 *	  (if (= n 0)
 *	 		'derp
 *	 		(begin
 *			  (list 1 2 3 4 5 6 7 8 9 10)
 *			  (loop (- n 1)))))
 *
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
	/* Expand list if needed */
	if (++len >= alloc) {
		alloc += ALLOC_EXPAND;
		ptrs = better_realloc(ptrs, alloc * sizeof(void *));
		mark = better_realloc(mark, alloc);
		if (!ptrs || !mark) {
			fatal_error_msg("marksweep: realloc has failed");
			exit(1);
		}
	}

	/* Add pointer to list and set initial zero-mark */
	mark[len - 1] = 0;
	ptrs[len - 1] = p;
}

void do_mark(void* p, int m)
{
	int i;

	/* Don't mark NULL */
	if (!p)
		return;

	/* 
	 * Look for the pointer in the
	 * list and mark it
	 */
	for (i = 0; i < len; ++i) {
		if (ptrs[i] == p) {
			mark[i] = m;
			return;
		}
	}
}

void gc()
{
	int i;
	int orig;

	void **copy_ptr;
	char *copy_mark;

	/* 
	 * Mark - this is the crucial step where everything
	 * still recursively visible starting from the
	 * global environment gets marked as non-garbage.
	 */
	marksweep(global);

	/* Sweep - free what hasn't been marked */
	for (i = 0; i < len; ++i)
		if (ptrs[i] != NULL)
			if (mark[i] == 0)
				free(ptrs[i]);

	/* 
	 * Subsequently, the garbage list is "compacted",
	 * which is to say that the freed stuff is removed
	 * from the object arrays
	 */

	/* Make a copy of the object lists (ptr, mark) */
	copy_ptr = malloc(len * sizeof(void *));
	if (!copy_ptr) {
		fatal_error_msg("marksweep: malloc failed");
		exit(1);
	}
	memcpy(copy_ptr, ptrs, len * sizeof(void *));
	
	copy_mark = malloc(len);
	memcpy(copy_mark, mark, len);

	/* Now rebuild the mark list with the nonfreed stuff */
	orig = len;
	len = 0;
	for (i = 0; i < orig; ++i) 
		if (copy_mark[i] != 0)
			add_ptr(copy_ptr[i]);

	/* Finally, erase the copies of the old list */
	free(copy_ptr);
	free(copy_mark);
}

/*
 * Mark a list object and related
 * children
 */
void marksweep_list(list_t *l)
{
	int i;

	/* Mark the list itself */
	do_mark(l, 1);
	
	/* Mark the list_t struct-fields */
	do_mark(l->c, 1);
	do_mark(l->head, 1);

	/* 
	 * Lists, closures, and conses have children;
	 * recurse onto them
	 */
	if (l->type == LIST || l->type == CLOSURE || l->type == CONS)
		for (i = 0; i < l->cc; ++i)
			marksweep_list(l->c[i]);

	/* Closures are associated with environments */
	if (l->type == CLOSURE && l->closure != global)
		marksweep(l->closure);
}

/*
 * Mark an environment object and
 * related children
 */
void marksweep(env_t *e)
{
	int i;

	/* Mark environment itself */
	do_mark(e, 1);
	
	/* Mark env_t struct-fields */
	do_mark(e->sym, 1);
	do_mark(e->ptr, 1);

	/* Mark symbol names */
	for (i = 0; i < e->count; ++i)
		do_mark(e->sym[i], 1);

	/* Mark symbol objects */
	for (i = 0; i < e->count; ++i)
		marksweep_list(e->ptr[i]);

	/* 
	 * Mark father environment, if any.
	 * do not remark global environment
	 * because it already gets marked
	 * first when gc() is called
	 * (avoiding an infinite loop ?...) 
	 */
	if (e->father && e->father != global)
		marksweep(e->father);
}
