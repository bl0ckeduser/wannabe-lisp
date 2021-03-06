#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* 
 * This could maybe done in a more 
 * sophisticated way,
 * (function pointers I guess)
 * but "oh well" 
 *
 * Yes, I know about hash tables and pointers even
 * fancier things.  No, I don't feel like using them
 * to make the code here fancier than a chain of
 * strcmp's. Yes, I wanted to learn about Scheme when
 * I wrote this project. No, I don't want to make
 * this a professional-quality ultrafast interpreter.
 *
 */

/*
 * Add primitive operator name to environment
 * This maps the symbol `sym' to a list
 * (PRIM-OP sym), which is recognized as the
 * evaluator as a special form which causes
 * it to run the routine "do_prim_op()",
 * which is defined lower down in this file.
 */
void add_primop(env_t *e, char *sym)
{
	list_t *l = new_list();
	l->type = LIST;
	add_child(l, mksym("PRIM-OP"));
	add_child(l, mksym(sym));
	env_add(e, sym, l);
}

/*
 * Install all the symbols for the primitive
 * operations into an environment
 */
void install_primitives(env_t *env)
{
	add_primop(env, "+");
	add_primop(env, "-");
	add_primop(env, "*");
	add_primop(env, "remainder");
	/* TODO: more arithmetic ops */

	add_primop(env, "=");
	add_primop(env, ">");
	add_primop(env, "<");
	add_primop(env, ">=");
	add_primop(env, "<=");
	add_primop(env, "not");

	add_primop(env, "car");
	add_primop(env, "cdr");
	add_primop(env, "cons");
	add_primop(env, "null?");

	add_primop(env, "display");
	add_primop(env, "newline");

	add_primop(env, "pair?");
	add_primop(env, "symbol?");
	add_primop(env, "eq?");
	add_primop(env, "number?");

	add_primop(env, "newline");
	
	add_primop(env, "save-to");
	add_primop(env, "load");

	add_primop(env, "cons2list");
	add_primop(env, "debuglog");

	add_primop(env, "reverse");
}

/* 
 * FIXME: apparently the proper Scheme behaviour
 * for interpretering booleans is #f => false,
 * anything else => true
 */

list_t* do_prim_op(char *name, list_t *args)
{
	int i = 0;
	int j;
	int val = 0;
	list_t *l1;
	list_t* nl = c_malloc(sizeof(list_t));
	char *buf;

	if (!strcmp(name, "+")) {
		val = 0;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != NUMBER) {
				error_msg("+ expects numbers");
				code_error();
			}
			val += args->c[i]->val;
		}
		nl->type = NUMBER;
		nl->val = val;
		return nl;
	}

	if (!strcmp(name, "-")) {
		if (args->cc == 1) {
			/* single argument: unary minus sign */
			if (args->c[0]->type != NUMBER) {
				error_msg("- expects numbers");
				code_error();
			}
			val = -args->c[0]->val;
		} else {
			/* otherwise, standard N-ary subtraction */
			for (i = 0; i < args->cc; ++i) {
				if (args->c[i]->type != NUMBER) {
					error_msg("- expects numbers");
					code_error();
				}
				if (i == 0)
					val = args->c[i]->val;
				else
					val -= args->c[i]->val;
			}
		}
		nl->val = val;
		nl->type = NUMBER;
		return nl;
	}

	if (!strcmp(name, "*")) {
		val = 1;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != NUMBER) {
				error_msg("* expects numbers");
				code_error();
			}
			val *= args->c[i]->val;
		}
		nl->type = NUMBER;
		nl->val = val;
		return nl;
	}

	if (!strcmp(name, "remainder")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg("`remainder' expects two numbers");
			code_error();
		}
		nl->type = NUMBER;
		nl->val = args->c[0]->val % args->c[1]->val;
		return nl;
	}

	if (!strcmp(name, "=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg("= expects two numbers");
			code_error();
		}
		return makebool(args->c[0]->val == args->c[1]->val);
	}

	if (!strcmp(name, ">")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg("> expects two numbers");
			code_error();
		}
		return makebool(args->c[0]->val > args->c[1]->val);
	}

	if (!strcmp(name, "<")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg("< expects two numbers");
			code_error();
		}
		return makebool(args->c[0]->val < args->c[1]->val);
	}

	if (!strcmp(name, "<=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg("<= expects two numbers");
			code_error();
		}
		return makebool(args->c[0]->val <= args->c[1]->val);
	}

	if (!strcmp(name, ">=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			error_msg(">= expects two numbers");
			code_error();
		}
		return makebool(args->c[0]->val >= args->c[1]->val);
	}

	if (!strcmp(name, "not")) {
		val = 0;
		if (args->cc != 1) {
				error_msg("`not' expects one argument");
				code_error();
		}
		/* r6rs.pdf section 11.8, page 47 */
		val = args->c[0]->type == BOOL && !args->c[i]->val;
		return makebool(val);
	}


	if (!strcmp(name, "cons")) {
		if (args->cc != 2) {
			error_msg("`cons' expects 2 arguments");
			code_error();
		}
		/* just return the list as-is for now */
		memcpy(nl, args, sizeof(list_t));
		nl->type = CONS;
		return nl;
	}

	if (!strcmp(name, "car")) {
		if (args->cc != 1) {
			error_msg("`car' expects 1 argument");
			code_error();
		}
		if (args->c[0]->type != CONS) {
			error_msg("`car' expects a linked-list");
			code_error();
		}
		if (args->c[0]->cc < 1) {
			error_msg("`car' has failed");
			code_error();
		}
		return args->c[0]->c[0];
	}

	if (!strcmp(name, "cdr")) {
		if (args->cc != 1) {
			error_msg("`cdr' expects 1 argument");
			code_error();
		}
		if (args->c[0]->type != CONS) {
			error_msg("`cdr' expects a linked-list");
			code_error();
		}
		if (args->c[0]->cc < 2) {
			error_msg("`cdr' has failed");
			code_error();
		}
		return args->c[0]->c[1];
	}

	if (!strcmp(name, "null?")) {
		return makebool(args->cc == 1 
			&& (
				((args->c[0]->type == SYMBOL && !strcmp(args->c[0]->head, "NIL"))
				|| (args->c[0]->type == CONS && args->c[0]->cc == 0))));
	}

	if (!strcmp(name, "display")) {
		buf = malloc(LINEBUFSIZ);
		if (!buf) {
			error_msg("malloc failed");
			code_error();
		}
		*buf = 0;
		printout(args->c[0], buf);
#ifdef JS_GUI
		c_writeback(buf);
#else
		printf("%s", buf);
#endif
		free(buf);
		return args->c[0];
	}

	if (!strcmp(name, "pair?")) {
		/* check for null first */
		j = args->cc == 1 
		&& (
			((args->c[0]->type == SYMBOL && !strcmp(args->c[0]->head, "NIL"))
			|| (args->c[0]->type == CONS && args->c[0]->cc == 0)));
		return makebool(!j  /* not null, then the rest */
			&& args->cc == 1 && args->c[0]->type == CONS);
	}

	/* the following bit deals with (eq? A B) --
	 * apparently, eq? checks if two things evaluate
	 * to the same memory pointer. but further hackery
	 * is sufficient to deal with the case (eq? 'foo 'foo) */
	if (!strcmp(name, "eq?")) {
		if (args->cc != 2) {
			error_msg("`eq?' expects two arguments");
			code_error();
		}
		return makebool(args->c[0] == args->c[1]
			|| (args->c[0]->type == SYMBOL 
				&& args->c[1]->type == SYMBOL
				&& !strcmp(args->c[0]->head, args->c[1]->head))
			/* (eq? 'NIL '()) => #t */
			|| (args->c[0]->type == SYMBOL && !strcmp(args->c[0]->head, "NIL")
			    && args->c[1]->type == CONS && args->c[1]->cc == 0)
			/* (eq? '() 'NIL) => #t */
			|| (args->c[1]->type == SYMBOL && !strcmp(args->c[1]->head, "NIL")
			    && args->c[0]->type == CONS && args->c[0]->cc == 0)
			/* (eq? '() '()) => #t */
			|| (args->c[0]->type == CONS && args->c[0]->cc == 0
			    && args->c[1]->type == CONS && args->c[1]->cc == 0));
	}

	if (!strcmp(name, "symbol?")) {
		return makebool(args->cc == 1 
			&& args->c[0]->type == SYMBOL);
	}

	if (!strcmp(name, "number?")) {
		return makebool(args->cc == 1 
			&& args->c[0]->type == NUMBER);
	}

	if (!strcmp(name, "newline")) {
#ifdef JS_GUI
		c_writeback_nl("");
#else
		puts("");
#endif
		return mksym("NIL");
	}

	if (!strcmp(name, "save-to")) {
		if (save_mode) {
			return mksym("NIL");
		}
		save_file = fopen(args->c[0]->head, "w");
		if (!save_file) {
			error_msg("failed to open file for writing");
			code_error();
		}
		save_mode = 1;
		return mksym("savefile-ok");
	}

	if (!strcmp(name, "load")) {
		buf = malloc(strlen(args->c[0]->head) + 1);
		if (!buf) {
			error_msg("malloc failed");
			code_error();
		}
		strcpy(buf, args->c[0]->head);
		load_code_from_file(buf);
		free(buf);
		return mksym("HERP-DERP");
	}

	if (!strcmp(name, "cons2list")) {
		return cons2list(args->c[0]);
	}

	if (!strcmp(name, "debuglog")) {
		stacktracer_barf();
		return mksym("herp-derp");
	}

	if (!strcmp(name, "reverse")) {
		/* r6rs.pdf, page 48 */
		if (args->c[0]->type == CONS)
			l1 = cons2list(args->c[0]);
		else if (args->c[0]->type == LIST)
			l1 = args->c[0];
		else return mksym("NIL");

		if (l1->cc == 0)
			return mksym("NIL");

		nl = new_list();
		nl->type = LIST;
		for (i = l1->cc - 1; i >= 0; --i)
			add_child(nl, l1->c[i]);
		return makelist(nl);
	}

	return NULL;
}
