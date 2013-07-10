#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

list_t* eval(list_t *l, env_t *env)
{
	extern list_t *new_list();
	env_ref_t er;
	list_t *argl, *proc;
	list_t *ev;
	list_t *nw, *nw2, *nw3;
	list_t *pred;
	int i;

	/* Deal with special forms (lambda, define, ...) first */
	/* TODO: cond, maybe other stuff ? */

	/* (lambda (arg-1 arg-2 ... arg-n) exp) */
	if (l->type == LIST && !strcmp(l->c[0]->head, "lambda")) {
		nw = new_list();
		nw->type = CLOSURE;
		nw->closure = env;
		for (i = 1; i < l->cc; ++i)
			add_child(nw, l->c[i]);
		return nw;
	}

	/* (define ...) family of special-forms */
	if (l->type == LIST && !strcmp(l->c[0]->head, "define")) {
		/* (define SYMBOL EXP) */
		if (l->cc == 3 && l->c[1]->type == SYMBOL) {
			env_add(env, l->c[1]->head, REF,
				(ev = eval(l->c[2], env)));
			return ev;
		/* 		(define (twice x) (+ x x))
		 * =>	(define twice (lambda (x) (+ x x)))
		 */ 
		} else if (l->cc == 3 && l->c[1]->type == LIST
			&& l->c[1]->cc && l->c[1]->c[0]->type == SYMBOL) {
			proc = l->c[1]->c[0];
			argl = new_list();
			for (i = 1; i < l->c[1]->cc; ++i)
				add_child(argl, l->c[1]->c[i]);
			nw = new_list();
			nw->type = CLOSURE;
			nw->closure = env;
			add_child(nw, argl);
			add_child(nw, l->c[2]);
			env_add(env, proc->head, REF,
				nw);
			return nw;
		} else {
			printf("Error: improper use of `define' special form\n");
			exit(1);
		}
	}

	/* if special form -- (if BOOL A B) */
	if (l->type == LIST && !strcmp(l->c[0]->head, "if")) {
		/* check arg count */
		if (l->cc != 4) {
			printf("Error: improper use of `if' special form\n");
			exit(1);
		}
		
		/* evaluate the predicate,
		 * ensuring it is of proper type */
		pred = eval(l->c[1], env);
		if (pred->type != BOOL) {
			printf("Error: boolean expected as 1st argument of `if'\n");
			exit(1);
		}

		/* return A or B depending upon predicate */
		if (pred->val) {
			return eval(l->c[2], env);
		} else {
			return eval(l->c[3], env);
		}
	}

	/* quote */
	if (l->type == LIST && l->cc == 2 && l->c[0]->type == SYMBOL
		&& !strcmp(l->c[0]->head, "QUOTE")) {
		return l->c[1];
	}

	/* 
	 * If the special cases don't match a list,
	 * it's a function application, e.g.
	 *		(+ 1 2 3)
 	 */
	if (l->type == LIST && l->cc) {
		/* group arguments */
		argl = new_list();
		argl->type = LIST;
		for (i = 1; i < l->cc; ++i)
			add_child(argl, l->c[i]);

		/* evaluate arguments */
		evlist(argl, env);

		/* evaluate procedure */
		proc = eval(l->c[0], env);

		/* apply procedure to arguments */
		return apply(proc, argl);
	}

	/* symbol */
	if (l->type == SYMBOL) {
		/* look up, climbing up environment chain */
		er = lookup(env, l->head);
		if (er.e == NULL) {
			printf("Error: unbound variable `%s'\n", l->head);
			exit(1);
		}
		/* it's a primitive operator */
		if (er.e->ty[er.i] == PRIM_OP) {
			nw = new_list();
			nw->type = LIST;
			add_child(nw, mksym("PRIM-OP"));
			nw2 = new_list();
			strcpy(nw2->head, l->head);
			add_child(nw, nw2);
			return nw;
		}
		/* general case */
		else if (er.e->ty[er.i] == REF)
			return er.e->ptr[er.i];
	}

	return l;
}

/*
 * Evalute each member of a list, in-place
 */
void evlist(list_t* l, env_t *env)
{
	extern list_t *eval(list_t *l, env_t *env);
	int i = 0;
	for (i = 0; i < l->cc; ++i)
		l->c[i] = eval(l->c[i], env);
}

