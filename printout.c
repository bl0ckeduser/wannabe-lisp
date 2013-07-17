#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

char *buf;

void printout(list_t *l, char *s)
{
	extern void printout_iter(list_t* l, int d, char* s);
	buf = malloc(128);
	printout_iter(l, 0, s);
	free(buf);
}

void printout_iter(list_t* l, int d, char *out)
{
	int i;

	/* switch based on type */
	if (l->type == NUMBER) {
		sprintf(buf, "%d", l->val);
		strcat(out, buf);
	} else if(l->type == SYMBOL) {
		sprintf(buf, "%s", l->head);
		strcat(out, buf);
	} else if (l->type == LIST) { 
		sprintf(buf, "(%s", l->head);
		strcat(out, buf);
		for (i = 0; i < l->cc; ++i) {
			if (i > 0) {
				sprintf(buf, " ");
				strcat(out, buf);
			}
			printout_iter(l->c[i], d + 1, out);
		}
		sprintf(buf, ")");
		strcat(out, buf);
	} else if (l->type == CLOSURE) {
		sprintf(buf, "#<CLOSURE:%p>", l->closure);
		strcat(out, buf);
	} else if (l->type == BOOL) {
		if (l->val)
			sprintf(buf, "#t");
		else
			sprintf(buf, "#f");
		strcat(out, buf);
	} else if (l->type == CONS) {
		if (l->cc == 2
			&& (l->c[1]->type == SYMBOL && !strcmp(l->c[1]->head, "NIL")
				|| (l->c[1]->type == CONS && l->c[1]->cc == 0))) {
			sprintf(buf, "(");
			strcat(out, buf);
			printout_iter(l->c[0], d + 1, out);
			sprintf(buf, ")");
			strcat(out, buf);
		}
		else
		if (l->cc == 2 && (l->c[1]->cc != 2 || l->c[1]->type != CONS)) {
			sprintf(buf, "(");
			strcat(out, buf);
			printout_iter(l->c[0], d + 1, out);
			/* (cons 'a '()) => (a)
			 * r6rs.pdf, section 11.9, page 48
			 */
			if (!(l->c[1]->type == CONS && l->c[1]->cc == 0)) {
				sprintf(buf, " . ");
				strcat(out, buf);
				printout_iter(l->c[1], d + 1, out);
			}
			sprintf(buf, ")");
			strcat(out, buf);
		} else if (l->cc) {
			sprintf(buf, "(");
			strcat(out, buf);
			while (1) {
				if (l->cc == 2 && l->c[1]->cc) {
					printout_iter(l->c[0], d + 1, out);
					sprintf(buf, " ");
					strcat(out, buf);
					l = l->c[1];
				} else {
					if (l->cc == 2
						&& (l->c[1]->type == SYMBOL && !strcmp(l->c[1]->head, "NIL")
						|| (l->c[1]->type == CONS && l->c[1]->cc == 0))) {
						printout_iter(l->c[0], d + 1, out);
					} else { 
						printout_iter(l, d + 1, out);
					}
					break;
				}
			}
			sprintf(buf, ")");
			strcat(out, buf);
		} else {
			sprintf(buf, "()");
			strcat(out, buf);
		}
	}
}


