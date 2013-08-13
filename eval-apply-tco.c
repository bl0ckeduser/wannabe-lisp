#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/*
 * This file contains the heart of the interpreter:
 * the routines "eval" and "apply". For reasons
 * of efficiency, eval and apply are actually
 * mishmashed into a single C function called
 * "eval_apply_tco". This makes it possible to
 * do tail-call optimization in a way that is
 * guaranteed to work regardless of whether the
 * C compiler used provides TCO itself: goto
 * statements.
 *
 * This implementation is based on SICP's metacircular 
 * evaluator eval/apply tutorial
 */

static char *buf;
static char *buf2;

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

/* And now, the eval/apply combined routine */

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

	/* TCO debugging stuff; owned by `tco_debug.c' */
	extern int frames;
	extern int frames_usage_max;
	extern void new_frame();
	extern void close_frame();

	/* 
	 * FIXME: apparently the proper Scheme behaviour
	 * for interpretering booleans is #f => false,
	 * anything else => true
	 */

	new_frame();

tco_iter:

	if (oper == 0) {

		/* ==================== EVAL =========================== */

		/* == printout current evaluation-step to the debugger = */
		if (l->type == LIST || l->type == CONS) {
			buf = malloc(1024);
			*buf = 0;
			printout(l, buf);
			stacktracer_push(buf);
			free(buf);
		}
		/* note that symbols get debug-tracked lower down, in the 
		 * part of the evaluator where symbols are evaluated */
		/* ===================================================== */

		/*
		 * It is necessary to deal with special forms 
		 * (lambda, define, ...) first. 
		 */

		/* Don't ask why, but () => (), at least
		 * according to both tinyscheme and MIT/GNU Scheme.
		 * I'm not sure what DA STANDARD has to say...
		 */
		if (l->type == LIST && l->cc == 0)
			return l;

		/* (apply proc arg1 arg2 ... argN) */
		if (l->type == LIST && !strcmp(l->c[0]->head, "apply")) {
			/* see r6rs.pdf, page 53 */
			/* arg1, arg2, ..., argN are evaluated and stored in `argl',
			 * then `append' is applied on these, giving nw; finally
			 * proc is applied on nw
			 */
			argl = new_list();
			argl->type = LIST;
			for (i = 2; i < l->cc; ++i)
				add_child(argl, l->c[i]);
			evlist(argl, env);
			nw = cons2list(call_apply(call_eval(mksym("append"), env), argl));
			TCO_apply(call_eval(l->c[1], env), nw);
		}

		/* (and ... ) with short-circuit 
		 * If it wasn't for the short-circuit, it wouldn't
		 * need to be a special form -- this code kicks away 
		 * the standard "eval all arguments, then apply"
		 * behaviour
		 */
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
		 * eval/apply stack frames used in the evaluation of EXP 
		 * 
		 * This is a custom extension not from the Scheme
		 * standard. Its purpose was for me to see if TCO
		 * worked. 
		 */
		if (l->type == LIST && !strcmp(l->c[0]->head, "max-space")) {
			frames_usage_max = 0;
			frames = 0;

			call_eval(l->c[1], env);

			nw = new_list();
			nw->type = NUMBER;
			nw->val = frames_usage_max;
			return nw;
		}

		/* (leval ...) => hook to eval(..., env)
		 * i.e. evaluate an expression in the current
		 * local environment.
		 *
		 * This is not part of the scheme standard,
		 * it's a custom extension. I think scheme has
		 * an `eval', though.
		 */
		if (l->type == LIST && !strcmp(l->c[0]->head, "leval")) {
			if (l->c[1]->type == LIST && l->c[1]->c[0]->type == SYMBOL
				&& !strcmp(l->c[1]->c[0]->head, "quote")) {
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
			 *  as in the `apply' routine at the bottom
			 *  of this file) */

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
				env_add(env, l->c[1]->head,
					(ev = call_eval(l->c[2], env)));
				close_frame();
				return ev;
			/* 		(define (twice x) (+ x x))
			 * =>	(define twice (lambda (x) (+ x x)))
			 */ 
			} else if (l->cc >= 3 && l->c[1]->type == LIST
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
				for (j = 2; j < l->cc; ++j)
					add_child(nw, l->c[j]);
				env_add(env, proc->head,
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

			env_set(env, l->c[1]->head,
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

			/* evaluate and return A or B depending upon predicate */
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

		/* (quote foo) => foo */
		if (l->type == LIST && l->cc == 2 && l->c[0]->type == SYMBOL
			&& !strcmp(l->c[0]->head, "quote")) {
			if (l->c[1]->type == SYMBOL) {
				close_frame();
				return l->c[1];
			} else if (l->c[1]->type == LIST) {
				close_frame();
				/* 
				 * Convert from internal linear representation
				 * to lisp car/cdr linked list form 
				 */
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
			proc = call_eval(l->c[0], env);

			/* apply procedure to arguments */
			TCO_apply(proc, argl);
		}

		/* symbol */
		if (l->type == SYMBOL) {
			/* look up, climbing up environment chain */
			er = lookup(env, l->head);

			/* lookup failed */
			if (er.e == NULL) {
				buf = malloc(1024);
				sprintf(buf, "unbound variable `%s'", l->head);
				error_msg(buf);
				free(buf);
				code_error();
			}

			/* ================================
			 * give the debugger's symbol table
			 * a printout of the symbol's value */
			buf = malloc(1024 * 2);
			if (!buf) {
				error_msg("malloc 2KB failed");
				code_error();
			}
			*buf = 0;
			printout(er.e->ptr[er.i], buf);
			stacktracer_push_sym(l->head, buf);
			free(buf);
			/* ================================ */		

			close_frame();
			return er.e->ptr[er.i];
		}

		close_frame();
		return l;

	} else {

		/* ==================== APPLY =========================== */

		env_t *ne;
		int i;

		/* Primitive operation */
		if (proc->cc == 2 && !strcmp(proc->c[0]->head, "PRIM-OP")) {
			close_frame();
			return do_prim_op(proc->c[1]->head, args);
		}

		/* General case */
		if (proc->type == CLOSURE) {
			/* Build a new environment */
			ne = new_env();
			ne->father = proc->closure;

			/* Bind the formal parameters 
			 * in the new environment */
			
			/* special case: whole arglist as single variable,
			 * as in (lambda x x) */
			if (proc->c[0]->type == SYMBOL) {
				env_add(ne, proc->c[0]->head,
					args->type == LIST ? makelist(args) : args);
			}
			else for (i = 0; i < proc->c[0]->cc; ++i) {
				/* special case: "." rest notation */
				if (!strcmp(proc->c[0]->c[i]->head, ".")) {
					if (i + 2 != proc->c[0]->cc) {
						error_msg("improper use of `.' in `lambda'");
						code_error();
					}
					nw = new_list();
					nw->type = LIST;
					for (j = i; j < args->cc; ++j)
						add_child(nw, args->c[j]);
					env_add(ne, proc->c[0]->c[i + 1]->head,
						makelist(nw));
					break;
				} else {
					/* general case */
					
					/* check for argcount mismatch */					
					if (i >= args->cc || i >= proc->c[0]->cc)
						goto afail;

					env_add(ne, proc->c[0]->c[i]->head,
						args->c[i]);
				}
			}
	
			/* Evaluate the bodies of the closure
			 * in the new environment which includes
			 * the bound parameters	*/
			for (i = 1; i < proc->cc - 1; ++i)
				call_eval(proc->c[i], ne);

			/* Return the evaluation of the last body */
			for (j = i; j < proc->cc - 1; ++j)
				call_eval(proc->c[j], ne);
			TCO_eval(proc->c[proc->cc - 1], ne);
		}

afail:
		error_msg("`apply' has failed");
		code_error();
	}

	close_frame();
	return NULL;
}

