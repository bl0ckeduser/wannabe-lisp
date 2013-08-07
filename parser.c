#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wannabe-lisp.h"

/* 
 * Table of abbreviations, as shown in
 * r6rs.pdf, section 4.3.5, page 17 
 */
struct {
	char *shorthand;
	char *internal;
} abbrev[] = {
	{"'", "quote"},
	{"`", "quasiquote"},
	{",@", "unquote-splicing"},
	{",", "unquote"},
	{"#'", "syntax"},
	{"#`", "quasisyntax"},
	{"#,@", "unsyntax-splicing"},
	{"#,", "unsyntax"},
	{NULL, NULL}};

char* build(list_t* l, char *expr)
{
	char *p, *q;
	char *old;
	int i;
	int lambda = 0;
	char* tok;
	int sgn;
	list_t* child;
	list_t* child2;
	int len;

	tok = malloc(32);
	p = expr;

	for (i = 0; abbrev[i].shorthand; ++i) {
		if (!strncmp(abbrev[i].shorthand, p, strlen(abbrev[i].shorthand))) {
			child = new_list();
			p = build(child, p + strlen(abbrev[i].shorthand));
			add_child(l, mksym(abbrev[i].internal));
			add_child(l, child);
			l->type = LIST;
			goto final;
		}
	}

	if (*p == '(') {	/* list */
		++p;
		l->type = LIST;
		l->cc = 0;
		i = 0;
		while (*p != ')' && *p) {
			while (*p && *p == ' ' || *p == '\t')
				++p;
			if (!*p)
				break;
			child = new_list();
			old = p;
			p = build(child, p);
			if (p == old)
				break;
			add_child(l, child);
		}

		if (*p++ != ')') {
			error_msg("Error: ) expected");
			code_error();
		}
	}
	else if (isnum(*p) || (*p == '-' && isalnum(*(p+1)))) {	/* number */
		q = tok;
		if (*p == '-') {
			sgn = -1;
			++p;
		} else {
			sgn = 1;
		}
		len = 0;
		while (*p && isnum(*p)) {
			*q++ = *p++;
			if (++len >= 32) {
				error_msg("numeral too long");
				code_error();
			}
		}
		*q = 0;
		l->type = NUMBER;
		sscanf(tok, "%d", &(l->val));
		l->val *= sgn;
	} else {	/* symbol */
		while (*p == ' ' || *p == '\t')
			++p;
		q = tok;
		len = 0;
		while (*p != '(' && *p != ')' && validname(*p)) {
			*q++ = *p++;
			if (++len >= 32) {
				error_msg("symbol name too long");
				code_error();
			}
		}
		*q = 0;
		l->type = SYMBOL;
		strcpy(l->head, tok);
	}

final:
	free(tok);
	return p;
}

