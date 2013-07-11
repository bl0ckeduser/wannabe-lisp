#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* Based on SICP's metacircular eval/apply tutorial */

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
	/* TODO: other stuff ? */

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

	/* cond special form --
	 * (cond (p1 e1) (p2 e2) ... (pN eN)) 
	 * the special predicate-symbol `else' always matches
	 */
	if (l->type == LIST & l->cc >= 2 && l->c[0]->type == SYMBOL
		&& !strcmp(l->c[0]->head, "cond")) {

		for (i = 1; i < l->cc; ++i) {
			if (l->c[i]->cc != 2) {
				printf("Error: arguments to `cond' should be"
					   " two-element lists...\n");
				exit(1);
			}
			/* deal with `else' special case */
			if (l->c[i]->c[0]->type == SYMBOL
				&& !strcmp(l->c[i]->c[0]->head, "else")) {
				return eval(l->c[i]->c[1], env);
			}
			/* general case */
			pred = eval(l->c[i]->c[0], env);
			if (pred->type != BOOL) {
				printf("Error: boolean expected in `cond'\n");
				exit(1);
			}
			if (pred->val)
				return eval(l->c[i]->c[1], env);
		}

		/* not sure what to return when nothing matches,
		 * I guess `nil' is reasonable ... */
		return mksym("NIL");
	}

	/* (list X Y Z ... ) */
	/* note that unlike for quoted lists, this involves evaluating
	 * the arguments before building the list */
	if (l->type == LIST && l->cc > 1 && l->c[0]->type == SYMBOL
		&& !strcmp(l->c[0]->head, "list")) {
		ev = new_list();
		for (i = 1; i < l->cc; ++i)
			add_child(ev, l->c[i]);
		evlist(ev, env);
		return makelist(ev);	/* cons-ify */
	}

	/* quote */
	if (l->type == LIST && l->cc == 2 && l->c[0]->type == SYMBOL
		&& !strcmp(l->c[0]->head, "QUOTE")) {
		if (l->c[1]->type == SYMBOL)
			return l->c[1];
		else if (l->c[1]->type == LIST) {
			/* cons-ify */
			return makelist(l->c[1]);
		}
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

/* 
 * (1 2 3 4)
 * => (1 . (2 . (3 . (4 . NIL))))
 */
list_t* makelist(list_t* argl)
{
	list_t *nw, *nw2, *nw3;
	list_t *cons;
	int i;
	
	nw3 = nw = new_list();
	nw->type = CONS;
	for (i = 0; i < argl->cc; ++i) {
		/* recursive application is necessary 
	 	 * so as to make possible cases like:
		 * 		]=> (caadr '(1 (1 2)))
		 * 		1	
		 */
		if (argl->c[i]->type == LIST) {
			cons = makelist(argl->c[i]);
			add_child(nw, cons);
		} else
			add_child(nw, argl->c[i]);
		if ((i + 1) < argl->cc) {
			nw2 = new_list();
			nw2->type = CONS;
			add_child(nw, nw2);
			nw = nw2;
		} else {
			/* put in a nil */
			add_child(nw, mksym("NIL"));
		}
	}
	return nw3;
}

