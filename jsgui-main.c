#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "wannabe-lisp.h"

/*
 * Lots of ugly glue between the JS/HTML
 * code and the C code. It's basically
 * a mishmash of bits of `main.c' and
 * `cli.c', with lots of hacks to prevent
 * strange issues that occur with emscripten.
 */

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

/* ============= JavaScript hooks ============= */

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

void c_writedebug(char *str)
{
	char *buf = malloc(1024 * 1024 * 2);
	sprintf(buf, "writedebug(\"%s\");", str);
	emscripten_run_script(buf);
	free(buf);
}

void c_write_char(char *str)
{
	char *buf = malloc(1024);
	sprintf(buf, "write_char(\"%s\");", str);
	emscripten_run_script(buf);
	free(buf);
}

/* ============================================ */

/*
 * JavaScript calls this when the C code
 * crashes (segfaults) or does an exit(1),
 * which it does when there are user code
 * errors
 */
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

	stacktracer_reset();
		
	c_writeback("]=> ");
	iter = 1;

	return 0;
}

/*
 * JavaScript calls this routine
 * after the user types a line
 * in the GUI terminal box.
 */
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

		/* deal with negative nest */
		if (bal < 0) {
			error_msg("Botched syntax");
			code_error();
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
				/* 
				 * `writechar', as opposed to `writeback',
				 * gets this included into the stuff
				 * that gets sent back to the C core,
				 * and also makes it to the clipboard
				 * when requested
				 */
				c_write_char(" ");
				c_write_char(" ");
				c_write_char(" ");
			}
		}
	}
	
	return 0;
}

int do_setup(int waste)
{
	c_writeback_nl("wannabe-lisp GUI mode has been started");

	/* setup stacktracer */
	c_writeback_nl("setting up debug tracer...");
	stacktracer_init();

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
	load_code_from_file("prefix.txt");

	c_writeback_nl("ready.");

	ready = 1;

	sprintf(buf, "");

	c_writeback("]=> ");
	++iter;

	/* clear debug goo from primitives/prefix installation */
	stacktracer_reset();

	return 0;
}
