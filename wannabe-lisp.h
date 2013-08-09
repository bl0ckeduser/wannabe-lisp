#ifndef WBLISP_H
#define WBLISP_H

#include <setjmp.h>

extern int interactive;
extern jmp_buf repl_jmp;
extern int save_mode;
extern FILE *save_file;

enum {
	SYMBOL = 0,
	NUMBER,
	LIST,
	CONS,
	CLOSURE,
	BOOL
};

enum {
	REF = 0
};

typedef struct env_ref {
	struct env* e;
	int i;
} env_ref_t;

typedef struct env {
	int count;
	int alloc;
	char **sym;
	char *ty;
	void **ptr;
	struct env *father;
} env_t;

typedef struct list {
	char* head;
	int type;
	int val;
	int cc;
	int ca;
	env_t *closure;
	struct list **c;
} list_t;

extern env_t *global;

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
extern void env_add(env_t *e, char *sym, int ty, void *p);
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
extern void env_set(env_t *e, char *sym, int ty, void *p);
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
