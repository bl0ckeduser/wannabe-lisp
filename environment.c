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

	e->count = 0;		/* number of symbols */
	e->father = NULL;	/* father environment */

	/* 
	 * e->sym, e->ptr are dynamic arrays,
	 * given an initial allocation of 8 elements
	 *
	 * e->sym:	symbol name
	 * e->ptr:	pointer to the data bound to the symbol
	 */
	e->alloc = 8;
	e->sym = c_malloc(e->alloc * sizeof(char *));
	e->ptr = c_malloc(e->alloc * sizeof(void *));

	return e;
}

/*
 * Look up a symbol in an environment-chain.
 * Most local references for a same symbol 
 * get returned first.
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
	 * Otherwise (recursively) try
	 * the father environment, else
	 * fall back on the default
	 * error-value of {0, NULL} 
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
void env_add(env_t *e, char *sym, void *p)
{
	int c = e->count;
	char **nsym;
	char *nty;
	void **nptr;
	int i;
	env_ref_t ref;

	/* 
	 * Check if the symbol that wants to
	 * be added has already been added 
	 */
	if ((lookup(e, sym)).e == e) {
		/* Yes; call env_set() instead */
		env_set(e, sym, p);
		return;
	}

	/* 
	 * Ugly run-of-the-mill dynamic list expansion,
	 * (the environment structure is a pair of arrays),
	 * with the particularity that realloc is not
	 * used because I can't figure out how to use
	 * it properly in conjunction with the GC 
	 */
	if (++(e->count) >= e->alloc) {
		e->alloc += 16;
		nsym = c_malloc(e->alloc * sizeof(char *));
		nptr = c_malloc(e->alloc * sizeof(void *));
		
		for (i = 0; i < e->alloc - 16; ++i) {
			nsym[i] = e->sym[i];
			nptr[i] = e->ptr[i];
		}

		e->sym = nsym;
		e->ptr = nptr;
	}

	/* Copy symbol string */
	e->sym[c] = c_malloc(strlen(sym) + 1);
	strcpy(e->sym[c], sym);

	/* Copy object pointer */
	e->ptr[c] = p;
}

/*
 * Modify a symbol's value, as in set!
 */
void env_set(env_t *e, char *sym, void *p)
{
	env_ref_t ref = lookup(e, sym);
	char *tmp;

	/* If the reference doesn't exist, abort */
	if (ref.e == NULL) {
		tmp = malloc(1024);
		sprintf(tmp, "unbound variable `%s'", sym);
		error_msg(tmp);
		free(tmp);
		code_error();
	}

	/* Copy symbol string */
	(ref.e)->sym[ref.i] = c_malloc(strlen(sym) + 1);
	strcpy((ref.e)->sym[ref.i], sym);

	/* Copy object pointer */
	(ref.e)->ptr[ref.i] = p;
}
