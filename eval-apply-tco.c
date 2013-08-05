#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* Based on SICP's metacircular eval/apply tutorial */

static char *buf;

/*
 * Evalute each member of a list, in-place
 */
void eatco_evlist(list_t* l, env_t *env)
{
	int i = 0;
	for (i = 0; i < l->cc; ++i)
		l->c[i] = call_eval(l->c[i], env);
}

/* 
 * (1 2 3 4)
 * => (1 . (2 . (3 . (4 . NIL))))
 * 
 * The former form is strictly an
 * internal representation, which
 * is easier to build and to 
 * parse to. 
 *
 * Lisp is supposed to use the
 * latter "car/cdr" form. So, 
 * before any list is actually used,
 * it gets transformed thusly.
 */
list_t* makelist(list_t* argl)
{
	list_t *nw, *nw2, *nw3;
	list_t *cons;
	int i;	

	nw3 = nw = new_list();
	nw->type = CONS;
	for (i = 0; i < argl->cc; ++i) {
		/* Recursive application is necessary 
	 	 * so as to ensure cases like:
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
			/* put in a nil at the end;
			 * it is understood as end-of-list
			 * marker */
			add_child(nw, mksym("NIL"));
		}
	}
	return nw3;
}

/* 
 * The frames stuff here is for use by the 
 * special procedure `max-space'; see
 * in the evaluator below 
 */

int frames = 0;
int frames_usage_max = 0;

void new_frame()
{
	++frames;
	if (frames > frames_usage_max) {
		frames_usage_max = frames;
	}
}

void close_frame()
{
	--frames;
}

/*
 * The following are macros that emulate
 * the syntax of function calls but that
 * merely jump back to the beggining of the
 * eval/apply hybrid procedure below.
 * This make TCO easily possible.
 */

#define TCO_eval(l_arg, e_arg)			\
	{					\
		l = l_arg;			\
		env = e_arg; 			\
		oper = 0;			\
		goto tco_iter;			\
	}					\

#define TCO_apply(p_arg, a_arg)			\
	{					\
		proc = p_arg;			\
		args = a_arg; 			\
		oper = 1;			\
		goto tco_iter;			\
	}					\

list_t* eval_apply_tco(
	int oper, 				/* 0: eval, otherwise, apply */
	list_t *a_l, env_t *a_env, 		/* eval */
	list_t *a_proc, list_t *a_args)		/* apply */
{
	extern list_t *new_list();
	env_ref_t er;
	list_t *argl;
	list_t *ev;
	list_t *nw, *nw2, *nw3;
	list_t *pred;
	env_t *ne;
	int i, j;
	int val;
	
	list_t *l = a_l;
	env_t *env = a_env;
	list_t *proc = a_proc;
	list_t *args = a_args;

	/* 
	 * FIXME: apparently the proper Scheme behaviour
	 * for interpretering booleans is #f => false,
	 * anything else => true
	 */

	new_frame();


tco_iter:

	if (oper == 0) {

		/* Deal with special forms (lambda, define, ...) first */

#ifdef JS_GUI
		buf = malloc(1024);
		*buf = 0;
		sprintf(buf, "eval: %p; %d; %s; %d", l, l->cc, l->type == SYMBOL ? l->head : "-", l->val);
		puts(buf);
		free(buf);
#endif

		/* Don't ask why, but () => () */
		if (l->type == LIST && l->cc == 0)
			return l;

		/* (and ... ) with short-circuit */
		if (l->type == LIST && !strcmp(l->c[0]->head, "and")) {
			val = 1;
			for (i = 1; i < l->cc; ++i) {
				ev = call_eval(l->c[i], env);
				if (ev->type != BOOL) {
					error_msg("`and' expects boolean arguments");
					code_error();
				}
				val &= ev->val;
				if (!ev->val)
					break;
			}
			return makebool(val);
		}

		/* (or ... ) with short-circuit */
		if (l->type == LIST && !strcmp(l->c[0]->head, "or")) {
			val = 0;
			for (i = 1; i < l->cc; ++i) {
				ev = call_eval(l->c[i], env);
				if (ev->type != BOOL) {
					error_msg("`or' expects boolean arguments");
					code_error();
				}
				val |= ev->val;
				if (ev->val)
					break;
			}
			return makebool(val);
		}

		/* (max-space EXP) gives the maximum number of 
		 * eval/apply stack frames used in the evaluation of EXP */
		if (l->type == LIST && !strcmp(l->c[0]->head, "max-space")) {
			frames_usage_max = 0;
			frames = 0;

			call_eval(l->c[1], env);

			nw = new_list();
			nw->type = NUMBER;
			nw->val = frames_usage_max;
			return nw;
		}

		/* (leval ...) => hook to eval(..., env) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "leval")) {
			if (l->c[1]->type == LIST && l->c[1]->c[0]->type == SYMBOL
				&& !strcmp(l->c[1]->c[0]->head, "QUOTE")) {
				TCO_eval(l->c[1]->c[1], env);
			} else {
				ev = call_eval(l->c[1], env);
				if (ev->type == SYMBOL) {
					TCO_eval(ev, env);
				} else {
					error_msg("`leval' expects a symbol");
				}
			}
		}

		/* (cons-stream a b) => (cons a (delay b)) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "cons-stream")) {
			nw = new_list();
			nw->type = CONS;
			add_child(nw, call_eval(l->c[1], env));

			nw2 = new_list();
			nw2->type = CLOSURE;
			nw2->closure = env;
			nw3 = new_list();
			nw3->type = LIST;
			add_child(nw2, nw3);
			add_child(nw2, l->c[2]);

			add_child(nw, nw2);

			close_frame();
			return nw;
		}

		/* (delay exp) => (lambda () exp) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "delay")) {
			nw = new_list();
			nw->type = CLOSURE;
			nw->closure = env;
			nw2 = new_list();
			nw2->type = LIST;
			add_child(nw, nw2);
			add_child(nw, l->c[1]);
			close_frame();
			return nw;
		}

		/* (lambda (arg-1 arg-2 ... arg-n) exp1 exp2 ... expN) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "lambda")) {
			nw = new_list();
			nw->type = CLOSURE;
			nw->closure = env;
			for (i = 1; i < l->cc; ++i)
				add_child(nw, l->c[i]);
			close_frame();
			return nw;
		}

		/* (begin exp1 exp2 ... expN) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "begin")) {
			for (i = 1; i < l->cc - 1; ++i)
				call_eval(l->c[i], env);
			TCO_eval(l->c[i], env);
		}

		/* (let ((v1 e1) (v2 e2) ... (vN eN)) exp1 exp2 ... expN) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "let")) {
			/* (quite similar to a lambda application,
			 *  as in apply.c) */

			if (l->cc < 3)
				goto bad_let;

			/* Build a new environment */
			ne = new_env();
			ne->father = env;

			/* Bind symbols to *evaluated* expressions */
			for (i = 0; i < l->c[1]->cc; ++i) {
				if (l->c[1]->c[i]->cc != 2)
					goto bad_let;
				env_add(ne, 
					l->c[1]->c[i]->c[0]->head,
					REF, 
					call_eval(l->c[1]->c[i]->c[1], env));
			}
	
			/* Evaluate bodies in new environment;
			 * return the result of evaluating the
			 * last one */
			for (i = 2; i < l->cc; ++i)
				ev = call_eval(l->c[i], ne);

			close_frame();
			return ev;

	bad_let:
			error_msg("improper use of `let' special form");
			code_error();
		}

		/* (define ...) family of special-forms */
		if (l->type == LIST && !strcmp(l->c[0]->head, "define")) {
			/* (define SYMBOL EXP) */
			if (l->cc == 3 && l->c[1]->type == SYMBOL) {
				env_add(env, l->c[1]->head, REF,
					(ev = call_eval(l->c[2], env)));
				close_frame();
				return ev;
			/* 		(define (twice x) (+ x x))
			 * =>	(define twice (lambda (x) (+ x x)))
			 */ 
			} else if (l->cc == 3 && l->c[1]->type == LIST
				&& l->c[1]->cc && l->c[1]->c[0]->type == SYMBOL) {
				proc = l->c[1]->c[0];
				argl = new_list();
				argl->type = LIST;
				for (i = 1; i < l->c[1]->cc; ++i)
					add_child(argl, l->c[1]->c[i]);
				nw = new_list();
				nw->type = CLOSURE;
				nw->closure = env;
				add_child(nw, argl);
				add_child(nw, l->c[2]);
				env_add(env, proc->head, REF,
					nw);
				close_frame();
				return nw;
			} else {
				error_msg("improper use of `define' special form");
				code_error();
			}
		}

		/* (set! var val) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "set!")) {
			/* check arg count */
			if (l->cc != 3) {
				error_msg("improper use of `set!' special form");
				code_error();
			}

			env_set(env, l->c[1]->head, REF,
				(ev = call_eval(l->c[2], env)));
	
			close_frame();
			return ev;
		}

		/* if special form -- (if BOOL A B) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "if")) {
			/* check arg count */
			if (l->cc < 3) {
				error_msg("improper use of `if' special form");
				code_error();
			}
		
			/* evaluate the predicate,
			 * ensuring it is of proper type */
			pred = call_eval(l->c[1], env);
			if (pred->type != BOOL) {
				error_msg("boolean expected as 1st argument of `if'");
				code_error();
			}

			/* return A or B depending upon predicate */
			if (pred->val) {
				TCO_eval(l->c[2], env);
			} else {
				if (l->cc == 4) {
					TCO_eval(l->c[3], env);
				} else {
					close_frame();
					return mksym("NIL");
				}
			}
		}

		/* cond special form --
		 * (cond (p1 e11 ... e1N) (p2 e21 ... e2N) ... (pN eN1 .. eNN)) 
		 * the special predicate-symbol `else' always matches
		 */
		if (l->type == LIST & l->cc >= 2 && l->c[0]->type == SYMBOL
			&& !strcmp(l->c[0]->head, "cond")) {

			for (i = 1; i < l->cc; ++i) {
				if (l->c[i]->cc < 2) {
					error_msg("arguments to `cond' should be"
						   " lists of at least two elements");
					code_error();
				}
				/* deal with `else' special case */
				if (l->c[i]->c[0]->type == SYMBOL
					&& !strcmp(l->c[i]->c[0]->head, "else")) {
					TCO_eval(l->c[i]->c[1], env);
				}
				/* general case */
				pred = call_eval(l->c[i]->c[0], env);
				if (pred->type != BOOL) {
					error_msg("boolean expected in `cond'");
					code_error();
				}
				if (pred->val) {
					for (j = 1; j < l->c[i]->cc - 1; ++j)
						call_eval(l->c[i]->c[j], env);
					TCO_eval(l->c[i]->c[j], env);
				}
			}

			/* not sure what to return when nothing matches,
			 * I guess `nil' is reasonable ... */
			close_frame();
			return mksym("NIL");
		}

		/* (list X Y Z ... ) */
		/* note that unlike for quoted lists, this involves evaluating
		 * the arguments before building the list */
		if (l->type == LIST && l->c[0]->type == SYMBOL
			&& !strcmp(l->c[0]->head, "list")) {
			ev = new_list();
			ev->type = LIST;
			for (i = 1; i < l->cc; ++i)
				add_child(ev, l->c[i]);
			eatco_evlist(ev, env);
			close_frame();
			return makelist(ev);	/* cons-ify */
		}

		/* quote */
		if (l->type == LIST && l->cc == 2 && l->c[0]->type == SYMBOL
			&& !strcmp(l->c[0]->head, "QUOTE")) {
			if (l->c[1]->type == SYMBOL) {
				close_frame();
				return l->c[1];
			} else if (l->c[1]->type == LIST) {
				/* cons-ify */
				close_frame();
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
			eatco_evlist(argl, env);

			/* evaluate procedure */
			proc = call_eval(l->c[0], env);

			/* apply procedure to arguments */
			TCO_apply(proc, argl);
		}

		/* symbol */
		if (l->type == SYMBOL) {
			/* look up, climbing up environment chain */
			er = lookup(env, l->head);
			if (er.e == NULL) {
				buf = malloc(1024);
				sprintf(buf, "unbound variable `%s'", l->head);
				error_msg(buf);
				free(buf);
				code_error();
			}
		
			close_frame();
			return er.e->ptr[er.i];
		}

		close_frame();
		return l;

	} else {

	/* ======================================================== */

		env_t *ne;
		list_t *last;
		int i;

		/* Primitive operation */
		if (proc->cc == 2 && !strcmp(proc->c[0]->head, "PRIM-OP")) {
			close_frame();
			return do_prim_op(proc->c[1]->head, args);
		}

		/* General case */
		if (proc->type == CLOSURE) {
			/* Check that parameter count matches */
			if (proc->c[0]->cc != args->cc) {
				error_msg("arg count mismatch in closure application");
				goto afail;
			}

			/* Build a new environment */
			ne = new_env();
			ne->father = proc->closure;

			/* Bind the formal parameters 
			 * in the new environment */
			for (i = 0; i < proc->c[0]->cc; ++i)
				env_add(ne, proc->c[0]->c[i]->head,
					REF, args->c[i]);
	
			/* Evaluate the bodies of the closure
			 * in the new environment which includes
			 * the bound parameters	*/
			for (i = 1; i < proc->cc - 1; ++i)
				call_eval(proc->c[i], ne);

			/* Return the evaluation of the last body */
			TCO_eval(proc->c[i], ne);
		}

	afail:
		error_msg("`apply' has failed");

		buf = malloc(1024);
		if (!buf)
			error_msg("malloc failed");

		*buf = 0;
		printout(proc, buf);
		puts(buf);

		*buf = 0;
		printout(args, buf);
		puts(buf);

		free(buf);

		code_error();
	}

	close_frame();
	return NULL;
}


