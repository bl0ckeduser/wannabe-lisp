#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* 
 * This could maybe done in a more 
 * sophisticated way, but "oh well" 
 *
 *
 */

/*
 * Add primitive operator name to environment
 */
void add_primop(env_t *e, char *sym)
{
	list_t *l = new_list();
	l->type = LIST;
	add_child(l, mksym("PRIM-OP"));
	add_child(l, mksym(sym));
	env_add(e, sym, REF, l);
}

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
	add_primop(env, "and");
	add_primop(env, "or");
	add_primop(env, "not");

	add_primop(env, "car");
	add_primop(env, "cdr");
	add_primop(env, "cons");
	add_primop(env, "null?");

	add_primop(env, "display");

	add_primop(env, "pair?");
	add_primop(env, "symbol?");
	add_primop(env, "eq?");
	add_primop(env, "number?");

	add_primop(env, "newline");
	
	add_primop(env, "save-to");
	add_primop(env, "load");
}

list_t* do_prim_op(char *name, list_t *args)
{
	int i = 0;
	int j;
	int val;
	list_t* nl = c_malloc(sizeof(list_t));
	char *buf;
	FILE *prefix;
	list_t *expr;

	if (!strcmp(name, "+")) {
		val = 0;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != NUMBER) {
				printf("Error: + expects numbers\n");
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
			if (args->c[0]->type != NUMBER) {
				printf("Error: - expects numbers\n");
				code_error();
			}
			val = -args->c[0]->val;
		} else {
			for (i = 0; i < args->cc; ++i) {
				if (args->c[i]->type != NUMBER) {
					printf("Error: - expects numbers\n");
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
				printf("Error: * expects numbers\n");
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
			printf("Error: `remainder' expects two numbers\n");
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
			printf("Error: = expects two numbers\n");
			code_error();
		}
		return makebool(args->c[0]->val == args->c[1]->val);
	}

	if (!strcmp(name, ">")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			code_error();
		}
		return makebool(args->c[0]->val > args->c[1]->val);
	}

	if (!strcmp(name, "<")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			code_error();
		}
		return makebool(args->c[0]->val < args->c[1]->val);
	}

	if (!strcmp(name, "<=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			code_error();
		}
		return makebool(args->c[0]->val <= args->c[1]->val);
	}

	if (!strcmp(name, ">=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			code_error();
		}
		return makebool(args->c[0]->val >= args->c[1]->val);
	}

	if (!strcmp(name, "and")) {
		val = 1;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != BOOL) {
				printf("Error: and expects boolean arguments\n");
				code_error();
			}
			val &= args->c[i]->val;
		}
		return makebool(val);
	}

	if (!strcmp(name, "or")) {
		val = 0;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != BOOL) {
				printf("Error: or expects boolean arguments\n");
				code_error();
			}
			val |= args->c[i]->val;
		}
		return makebool(val);
	}

	if (!strcmp(name, "not")) {
		val = 0;
		if (args->cc != 1) {
				printf("Error: `not' expects one argument\n");
				code_error();
		}
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != BOOL) {
				printf("Error: `not' expects boolean arguments\n");
				code_error();
			}
			val = !(args->c[i]->val);
		}
		return makebool(val);
	}


	if (!strcmp(name, "cons")) {
		if (args->cc != 2) {
			printf("Error: `cons' expects 2 arguments\n");
			code_error();
		}
		/* just return the list as-is for now */
		memcpy(nl, args, sizeof(list_t));
		nl->type = CONS;
		return nl;
	}

	if (!strcmp(name, "car")) {
		if (args->cc != 1) {
			printf("Error: `car' expects 1 argument\n");
			code_error();
		}
		if (args->c[0]->type != CONS) {
			printf("Error: `car' expects a linked-list\n");
			code_error();
		}
		if (args->c[0]->cc < 1) {
			printf("Error: `car' has failed\n");
			code_error();
		}
		return args->c[0]->c[0];
	}

	if (!strcmp(name, "cdr")) {
		if (args->cc != 1) {
			printf("Error: `cdr' expects 1 argument\n");
			code_error();
		}
		if (args->c[0]->type != CONS) {
			printf("Error: `cdr' expects a linked-list\n");
			code_error();
		}
		if (args->c[0]->cc < 2) {
			printf("Error: `cdr' has failed\n");
			code_error();
		}
		return args->c[0]->c[1];
	}

	if (!strcmp(name, "null?")) {
		return makebool(args->cc == 1 
			&& (
				(args->c[0]->type == SYMBOL && !strcmp(args->c[0]->head, "NIL")
				|| (args->c[0]->type == CONS && args->c[0]->cc == 0))));
	}

	if (!strcmp(name, "display")) {
		buf = malloc(1024);
		*buf = 0;
		printout(args->c[0], buf);
		puts(buf);
		free(buf);
		return args->c[0];
	}

	if (!strcmp(name, "pair?")) {
		return makebool(args->cc == 1 
			&& args->c[0]->type == CONS);
	}

	/* the following bit deals with (eq? A B) --
	 * apparently, eq? checks if two things evaluate
	 * to the same memory pointer. but further hackery
	 * is sufficient to deal with the case (eq? 'foo 'foo) */
	if (!strcmp(name, "eq?")) {
		if (args->cc != 2) {
			printf("`eq?' expects two arguments\n");
			code_error();
		}
		return makebool(args->c[0] == args->c[1]
			|| (args->c[0]->type == SYMBOL 
				&& args->c[1]->type == SYMBOL
				&& !strcmp(args->c[0]->head, args->c[1]->head)));
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
		puts("");
		return mksym("NIL");
	}

	if (!strcmp(name, "save-to")) {
		if (save_mode) {
			return mksym("NIL");
		}
		save_file = fopen(args->c[0]->head, "w");
		if (!save_file) {
			printf("Error: failed to open file for writing\n");
		}
		save_mode = 1;
		return mksym("savefile-ok");
	}

	if (!strcmp(name, "load")) {
		if ((prefix = fopen(args->c[0]->head, "r"))) {
			buf = malloc(1024 * 1024 * 2);
			if (!buf) {
				printf("Error: load: malloc failed\n");
				code_error();
			}
			while (1) {
				*buf = 0;	
				do_read_file(buf, prefix, 1);
				if (!*buf || feof(prefix))
					break;
				if (*buf && *buf != ';') {
					expr = new_list();
					build(expr, buf);
					call_eval(expr, global);

					/* clean up for the next iteration */
					gc();
					sprintf(buf, "");
				}
			}
			fclose(prefix);
			free(buf);
		} else {
			printf("Note: couldn't load `%s'\n", args->c[0]->head);
		}
		return mksym("load-ok");
	}

	return NULL;
}
