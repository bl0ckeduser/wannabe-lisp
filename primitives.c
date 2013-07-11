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

void install_primitives(env_t *env)
{
	env_add(env, "+", PRIM_OP, NULL);
	env_add(env, "-", PRIM_OP, NULL);
	env_add(env, "*", PRIM_OP, NULL);
	env_add(env, "remainder", PRIM_OP, NULL);
	/* TODO: more arithmetic ops */

	env_add(env, "=", PRIM_OP, NULL);
	env_add(env, "=", PRIM_OP, NULL);
	env_add(env, ">", PRIM_OP, NULL);
	env_add(env, "<", PRIM_OP, NULL);
	env_add(env, ">=", PRIM_OP, NULL);
	env_add(env, "<=", PRIM_OP, NULL);
	env_add(env, "and", PRIM_OP, NULL);
	env_add(env, "or", PRIM_OP, NULL);
	env_add(env, "not", PRIM_OP, NULL);

	env_add(env, "car", PRIM_OP, NULL);
	env_add(env, "cdr", PRIM_OP, NULL);
	env_add(env, "cons", PRIM_OP, NULL);
	env_add(env, "null?", PRIM_OP, NULL);

	env_add(env, "display", PRIM_OP, NULL);
}

list_t* do_prim_op(char *name, list_t *args)
{
	int i = 0;
	int j;
	int val;
	list_t* nl = malloc(sizeof(list_t));

	if (!strcmp(name, "+")) {
		val = 0;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != NUMBER) {
				printf("Error: + expects numbers\n");
				exit(1);
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
				exit(1);
			}
			val = -args->c[0]->val;
		} else {
			for (i = 0; i < args->cc; ++i) {
				if (args->c[i]->type != NUMBER) {
					printf("Error: - expects numbers\n");
					exit(1);
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
				exit(1);
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
			exit(1);
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
			exit(1);
		}
		return makebool(args->c[0]->val == args->c[1]->val);
	}

	if (!strcmp(name, ">")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			exit(1);
		}
		return makebool(args->c[0]->val > args->c[1]->val);
	}

	if (!strcmp(name, "<")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			exit(1);
		}
		return makebool(args->c[0]->val < args->c[1]->val);
	}

	if (!strcmp(name, "<=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			exit(1);
		}
		return makebool(args->c[0]->val <= args->c[1]->val);
	}

	if (!strcmp(name, ">=")) {
		if (args->cc != 2
		|| args->c[0]->type != NUMBER
		|| args->c[1]->type != NUMBER) {
			printf("Error: = expects two numbers\n");
			exit(1);
		}
		return makebool(args->c[0]->val >= args->c[1]->val);
	}

	if (!strcmp(name, "and")) {
		val = 1;
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != BOOL) {
				printf("Error: and expects boolean arguments\n");
				exit(1);
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
				exit(1);
			}
			val |= args->c[i]->val;
		}
		return makebool(val);
	}

	if (!strcmp(name, "not")) {
		val = 0;
		if (args->cc != 1) {
				printf("Error: `not' expects one argument\n");
				exit(1);
		}
		for (i = 0; i < args->cc; ++i) {
			if (args->c[i]->type != BOOL) {
				printf("Error: `not' expects boolean arguments\n");
				exit(1);
			}
			val = !(args->c[i]->val);
		}
		return makebool(val);
	}


	if (!strcmp(name, "cons")) {
		if (args->cc != 2) {
			printf("Error: `cons' expects 2 arguments\n");
			exit(1);
		}
		/* just return the list as-is for now */
		memcpy(nl, args, sizeof(list_t));
		nl->type = CONS;
		return nl;
	}

	if (!strcmp(name, "car")) {
		if (args->cc != 1) {
			printf("Error: `car' expects 1 argument\n");
			exit(1);
		}
		if (args->c[0]->type != CONS) {
			printf("Error: `car' expects a linked-list\n");
			exit(1);
		}
		if (args->c[0]->cc < 1) {
			printf("Error: `car' has failed\n");
			exit(1);
		}
		return args->c[0]->c[0];
	}

	if (!strcmp(name, "cdr")) {
		if (args->cc != 1) {
			printf("Error: `cdr' expects 1 argument\n");
			exit(1);
		}
		if (args->c[0]->type != CONS) {
			printf("Error: `cdr' expects a linked-list\n");
			exit(1);
		}
		if (args->c[0]->cc < 2) {
			printf("Error: `cdr' has failed\n");
			exit(1);
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
		printout(args->c[0]);
		printf("\n");
		return args->c[0];
	}

	return NULL;
}
