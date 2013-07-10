#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

char* build(list_t* l, char *expr)
{
	char *p, *q;
	char *old;
	int i;
	int lambda = 0;
	char tok[16];
	list_t* child;
	list_t* child2;

	p = expr;

	if (*p == '\'') {	/* quoting */
		child = new_list();
		p = build(child, p + 1);
		add_child(l, mksym("QUOTE"));
		add_child(l, child);
		l->type = LIST;
	}
 	else if (*p == '(') {	/* list */
		++p;
		l->type = LIST;

		i = 0;
		while (*p != ')' && *p) {
			while (*p == ' ' || *p == '\t')
				++p;
			child = new_list();
			old = p;
			p = build(child, p);
			if (p == old)
				break;
			add_child(l, child);
		}

		if (*p++ != ')') {
			printf("\nError: ) expected\n");
			exit(1);
		}
	}
	else if (isnum(*p)) {	/* number */
		q = tok;
		while (*p && isnum(*p))
			*q++ = *p++;
		*q = 0;
		child = new_list();
		child->type = NUMBER;
		sscanf(tok, "%d", &(child->val));
		memcpy(l, child, sizeof(list_t));
	} else {	/* symbol */
		while (*p == ' ' || *p == '\t')
			++p;
		q = tok;
		while (*p != '(' && *p != ')' && validname(*p))
			*q++ = *p++;
		*q = 0;
		l->type = SYMBOL;
		strcpy(l->head, tok);
	}

	return p;
}

