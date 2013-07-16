#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * Make a new environment object
 */
env_t* new_env(void)
{
	env_t *e = c_malloc(sizeof(env_t));

	e->count = 0;
	e->alloc = 8;
	e->sym = c_malloc(e->alloc * sizeof(char *));
	e->ty = c_malloc(e->alloc * sizeof(char));
	e->ptr = c_malloc(e->alloc * sizeof(void *));
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
void env_add(env_t *e, char *sym, int ty, void *p)
{
	int c = e->count;
	char **nsym;
	char *nty;
	void **nptr;
	int i;
	env_ref_t ref;

	/* check if it's already there */
	if ((lookup(e, sym)).e == e) {
		/* yes; call env_set() instead */
		env_set(e, sym, ty, p);
		return;
	}

	if (++(e->count) >= e->alloc) {
		e->alloc += 16;
		nsym = c_malloc(e->alloc * sizeof(char *));
		nty = c_malloc(e->alloc);
		nptr = c_malloc(e->alloc * sizeof(void *));
		
		for (i = 0; i < e->alloc - 16; ++i) {
			nsym[i] = e->sym[i];
			nty[i] = e->ty[i];
			nptr[i] = e->ptr[i];
		}

		e->sym = nsym;
		e->ty = nty;
		e->ptr = nptr;
	}

	e->sym[c] = c_malloc(strlen(sym) + 1);
	strcpy(e->sym[c], sym);

	e->ty[c] = ty;

	e->ptr[c] = p;
}

/*
 * Modify a symbol's value, as in set!
 */
void env_set(env_t *e, char *sym, int ty, void *p)
{
	env_ref_t ref = lookup(e, sym);

	if (ref.e == NULL) {
		printf("Error: unbound variable `%s'\n", sym);
		code_error();
	}

	(ref.e)->sym[ref.i] = c_malloc(strlen(sym) + 1);
	strcpy((ref.e)->sym[ref.i], sym);

	(ref.e)->ty[ref.i] = ty;

	(ref.e)->ptr[ref.i] = p;
}
