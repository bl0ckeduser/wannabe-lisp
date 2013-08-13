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

/* Checks if a character can be reasonably
 * part of a symbol-name */
int symbol_name_char(char c)
{
	return
		c != '('
	&&	c != ')'
	&&	c != ' '
	&&	c != '\t'
	&&	c != '\n';
}

/*
 * Recursive parsing function.
 * Returns pointer to last-parsed character
 * in `expr'. 
 * 
 * `l' has to be allocated prior
 * to calling build()
 */
char* build(list_t* l, char *expr)
{
	char *p, *q;
	char *old;
	char* tok;
	int sgn;
	list_t* child;
	int len;
	int i;

	tok = malloc(32);
	p = expr;

	/* Deal with abbreviations
	 * e.g. 'x => (quote x)
	 * using the `abbrev' entry {"'", "quote"}
	 */
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

	/* Parse a list: (... */
	if (*p == '(') {
		++p;			/* eat ( */
		l->type = LIST;	/* setup list-object */
		l->cc = 0;		/* initialize child count */
		/* ======================================= */
		while (*p != ')' && *p) {
			/* consume whitespace */
			while (*p && (*p == ' ' || *p == '\t'))
				++p;
			/* stop if there's nothing left now */
			if (!*p)
				break;
			/* try to parse a list-child */
			child = new_list();
			old = p;
			p = build(child, p);
			/* check if list-child-parsing failed */
			if (p == old)
				break;	/* yes; stop */
			/* no; add child to the list, and continue */
			add_child(l, child);
		}
		/* ======================================= */
		if (*p++ != ')') {
			error_msg("Error: ) expected");
			code_error();
		}
	}
	/* Parse a number: 123, -123 */
	else if (isnum(*p) || (*p == '-' && isalnum(*(p+1)))) {	
		/* Eat up the sign if there is one, and set
		 * `sgn' accordingly' */
		if (*p == '-') {
			sgn = -1;
			++p;
		} else {
			sgn = 1;
		}
		/* ======================================= */
		q = tok;	/* use `tok' heapbuffer as copy dest */
		len = 0;	/* used to check if symbolname is too long */
		while (*p && isnum(*p)) {
			*q++ = *p++;
			if (++len >= 32) {
				error_msg("numeral too long");
				code_error();
			}
		}
		*q = 0;		/* null-terminate the string */
		/* ======================================= */
		/* Make the number object */
		l->type = NUMBER;
		sscanf(tok, "%d", &(l->val));
		l->val *= sgn;
	/* Anything else is probably a symbol */
	} else {
		/* Well, maybe there's some whitespace first,
		 * eat that up first */
		while (*p == ' ' || *p == '\t')
			++p;
		/* ======================================= */
		q = tok;	/* use `tok' heapbuffer as copy dest */
		len = 0;	/* used to check if symbolname is too long */
		while (*p && symbol_name_char(*p)) {
			*q++ = *p++;
			if (++len >= 32) {
				error_msg("symbol name too long");
				code_error();
			}
		}
		*q = 0;		/* null-terminate the string */
		/* ======================================= */
		/* Make the symbol object */
		l->type = SYMBOL;
		strcpy(l->head, tok);
	}

	/*
	 * "tok" is a heap buffer, so it must be freed.
	 * i'm not using a stack one because emscripten
	 * behaves erratically when these are used.
	 */
final:
	free(tok);
	return p;
}

