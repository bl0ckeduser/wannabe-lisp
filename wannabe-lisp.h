#ifndef WBLISP_H
#define WBLISP_H

#include <setjmp.h>

/* REPL stuff, see main.c */
extern int interactive;
extern jmp_buf repl_jmp;

/* REPL logging */
extern int save_mode;
extern FILE *save_file;

/*
 * Types of objects. used by field `type'
 * of `list_t'
 */
enum {
	SYMBOL = 0,
	NUMBER,
	LIST,
	CONS,
	CLOSURE,
	BOOL
};

/*
 * Structure returned by lookup()
 * routine in file environment.c
 *
 * Not sure why this isn't simply
 * replaced by the pointer e + i
 * aka &e[i]
 */
typedef struct env_ref {
	struct env* e;
	int i;
} env_ref_t;

/*
 * Environment data structure
 */
typedef struct env {
	int count;		/* number of symbols */
	int alloc;		/* allocated space for symbols */
	char **sym;		/* symbol names */
	void **ptr;		/* symbol bound-object pointers */
	struct env *father;	/* father environment */
} env_t;

/* 
 * The global environment 
 * (must be defined below env_t definition,
 * at least with my compiler ... 
 */
extern env_t *global;

/*
 * List data structure
 */
typedef struct list {
	char* head;		/* used for symbol names */
	int type;		/* type; see enum at top of file */
	int val;		/* used for bools (?) and ints */
	env_t *closure;		/* closure environment (for procedures) */

	int cc;			/* child count (for lists/conses) */
	int ca;			/* child allocation */
	struct list **c;	/* array of children */
} list_t;

extern list_t* eval(list_t *l, env_t *env);
extern void add_child(list_t *parent, list_t* child);
extern void printout(list_t *l, char *s);
extern list_t *new_list(void);
extern list_t* do_prim_op(char *name, list_t *args);
extern env_t* new_env(void);
extern list_t* apply(list_t *proc, list_t *args);
extern env_ref_t lookup(env_t *e, char *sym);
extern void evlist(list_t* l, env_t *env);
extern list_t* makebool(int cbool);
extern void env_add(env_t *e, char *sym, void *p);
extern void install_primitives(env_t *env);

extern char* build(list_t* l, char *expr);
extern int isnum(char c);
extern int validname(char c);
extern list_t* mksym(char *s);
extern void *c_malloc(long size);
extern void *c_realloc(void *ptr, long size);
extern void strip_nl(char *s);
extern list_t* makelist(list_t* argl);
extern void marksweep(env_t *e);
extern void add_ptr(void *p);
extern void gc();
extern void do_mark(void* p, int m);
extern void gc_selfdestroy();
extern void env_set(env_t *e, char *sym, void *p);
extern list_t* eval_apply_tco(int oper, list_t *a_l, env_t *a_env, list_t *a_proc, list_t *a_args);

#define call_eval(l, e) eval_apply_tco(0, l, e, NULL, NULL)
#define call_apply(p, a) eval_apply_tco(1, NULL, NULL, p, a)

extern list_t *copy_list(list_t *l);
extern int code_error();
extern int do_read_file(char *buf, FILE *f, int silent);
extern int check_comment(char *s);
extern void error_msg(char *s);
extern void fatal_error_msg(char *s);
extern void load_code_from_file(char *fil);
extern list_t* cons2list(list_t *);

extern void stacktracer_reset();
extern void stacktracer_init();
extern void stacktracer_destroy();
extern void stacktracer_push(char *s);
extern void stacktracer_show(char *s);
extern void stacktracer_prebarf();
extern void stacktracer_barf();
extern void stacktracer_push_sym(char *symb, char *prnt);

#endif
