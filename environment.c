#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * Make a new environment object
 */
env_t* new_env(void)
{
	env_t *e = malloc(sizeof(env_t));

	e->count = 0;
	e->alloc = 16;
	e->sym = malloc(16 * sizeof(char *));
	e->ty = malloc(16 * sizeof(char));
	e->ptr = malloc(16 * sizeof(void *));
	e->father = NULL;

	return e;
}

/*
 * Look up a symbol in an environment-chain
 */
env_ref_t lookup(env_t *e, char *sym)
{
	int i;
	env_ref_t ret;

	/* 
	 * Try to find the symbol in the
	 * current environment 
	 */
	for (i = 0; i < e->count; ++i) {
		if (!strcmp(e->sym[i], sym)) {
			ret.i = i;
			ret.e = e;
			return ret;
		}
	}

	/* 
	 * Otherwise try father env,
	 * falling back on the default
	 * error-value {0, NULL} 
	 */
	ret.i = 0;
	ret.e = NULL;
	if (e->father)
		ret = lookup(e->father, sym);

	return ret;
}

/*
 * Add a symbol to an environment
 */
int env_add(env_t *e, char *sym, int ty, void *p)
{
	int c = e->count;

	if (++(e->count) > e->alloc) {
		e->alloc += 16;
		e->sym = realloc(e->sym,
			e->alloc * sizeof(char *));
		e->ty = realloc(e->ty,
			e->alloc * sizeof(char));
		e->ptr = realloc(e->ptr,
			e->alloc * sizeof(void *));
	}

	e->sym[c] = malloc(strlen(sym) + 1);
	strcpy(e->sym[c], sym);

	e->ty[c] = ty;

	e->ptr[c] = p;
}
