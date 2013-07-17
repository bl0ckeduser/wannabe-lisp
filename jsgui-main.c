#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "wannabe-lisp.h"

int interactive = 1;
env_t *global;		/* pointer to global environment */
jmp_buf repl_jmp;	/* jump pointer to REPL used to 
					 * recover from in-code errors */
int save_mode = 0;
FILE *save_file;

static char *buf;

char* out;
char *p;
list_t *expr;
int i, c;
char *ptr, *old;
FILE *prefix;
int ready = 0;

int bal = -1;
int iter = 0;

/*
	function writeback(str)	=> c_writeback(char *)
	function writeback_nl(str) => c_writeback_nl(char *)
*/

void c_writeback(char *str)
{
	char *buf = malloc(1024);
	sprintf(buf, "writeback(\"%s\");", str);
	emscripten_run_script(buf);
	free(buf);
}

void c_writeback_nl(char *str)
{
	char *buf = malloc(1024);
	sprintf(buf, "writeback_nl(\"%s\");", str);
	emscripten_run_script(buf);
	free(buf);
}

int jsgui_error_handler(int derp)
{
	/* I have to do this reallocation thing
	 * for some reason,
	 * else the emscripten-ified code fucks up */
	free(buf);
	buf = malloc(1024 * 1024 * 2);
	sprintf(buf, "");

	bal = -1;
	gc();
		
	c_writeback("]=> ");
	iter = 1;

	return 0;
}

int handle_gui_line(char *lin)
{
	extern int do_setup(int waste);
	int ind;

	printf("'%s'\n", lin);

	/* initialize if necessary */
	if (!ready)
		do_setup(0);

	/* ignore empty lines, comments */
	if (!*lin || check_comment(lin)) {
		if (iter == 1)
			--iter;
	}
	else {
		printf("strcat '%s' to '%s'\n", lin, buf);
		strcat(buf, lin);
		strcat(buf, " ");
		printf("parencheck: '%s'\n", buf);
		/* check paren balance */
		bal = 0;
		for (p = buf; *p; ++p) {
			if (*p == '(')
				++bal;
			else if (*p == ')')
				--bal;
		}
		printf("pcheck => %d\n", bal);
	}

	/* if expression is done, do eval-print */
	if (bal == 0 && strlen(buf) >= 1) {
		/* parse input into a tree */
		expr = new_list();
		printf("build: '%s'\n", buf);
		build(expr, buf);

		/* eval and print */
		*out = 0;
		printout(call_eval(expr, global), out);
		c_writeback_nl(out);

		/* clean up for the next iteration */
		gc();
		/* I have to do this reallocation thing
		 * for some reason,
		 * else the emscripten-ified code fucks up */
		free(buf);
		buf = malloc(1024 * 1024 * 2);
		sprintf(buf, "");
		iter = 0;
		bal = -1;
	}

	/* =================== */

	/* Print prompt */
	if (!iter++)
		c_writeback("]=> ");
	else {
		/* new user line, missing parentheses */
		c_writeback("... ");
		/* auto-indent */
		if (bal > 0) {
			for (ind = 0; ind < bal; ++ind) {
				c_writeback("   ");
			}
		}
	}
	
	return 0;
}

int do_setup(int waste)
{
	c_writeback_nl("wannbe-lisp GUI mode has been started");

	c_writeback_nl("allocating buffers...");
	buf = malloc(1024 * 1024 * 2);
	out = malloc(1024 * 1024 * 2);

	if (!buf || !out) {
		c_writeback_nl("jsgui-main.c: setup: malloc() failed");
		exit(0);
	}

	/* safety herp derp */
	sprintf(buf, "");

	/* Create global environment */
	c_writeback_nl("creating global environment...");
	global = new_env();

	/* Set up the built-in procedure symbols 
	 * for arithmetic, comparisons, etc. */
	c_writeback_nl("installing primitives...");
	install_primitives(global);

	/* Load up some composite primitives.
	 * `prefix.txt' gets generated by `gen-prefix.py',
	 * see the latter for further details about
	 * these primitives */
	if ((prefix = fopen("prefix.txt", "r"))) {
		c_writeback_nl("loading prefix...");
		while ((fgets(buf, 1024, prefix))) {
			strip_nl(buf);
			if (*buf && !check_comment(buf)) {
				puts(buf);
				expr = new_list();
				build(expr, buf);

				call_eval(expr, global);

				/* clean up for the next iteration */
				gc();
				sprintf(buf, "");
			}
		}
		fclose(prefix);
	} else {
		c_writeback_nl("failed to load prefix, oh well.");
	}

	c_writeback_nl("ready.");

	ready = 1;

	sprintf(buf, "");

	c_writeback("]=> ");
	++iter;

	return 0;
}
