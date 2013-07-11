#ifndef WBLISP_H
#define WBLISP_H

extern int interactive;

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
	char head[32];
	int type;
	int val;
	int cc;
	int ca;
	env_t *closure;
	struct list **c;
} list_t;

extern list_t* eval(list_t *l, env_t *env);
extern void add_child(list_t *parent, list_t* child);
extern void printout(list_t *l);
extern list_t *new_list(void);
extern list_t* do_prim_op(char *name, list_t *args);
extern env_t* new_env(void);
extern list_t* apply(list_t *proc, list_t *args);
extern env_ref_t lookup(env_t *e, char *sym);
extern void evlist(list_t* l, env_t *env);
extern list_t* makebool(int cbool);
extern int env_add(env_t *e, char *sym, int ty, void *p);
extern void install_primitives(env_t *env);
extern void do_read(char *buf);
extern char* build(list_t* l, char *expr);
extern int isnum(char c);
extern int validname(char c);
extern list_t* mksym(char *s);
extern void *c_malloc(long size);
extern void *c_realloc(void *ptr, long size);
extern void strip_nl(char *s);
extern list_t* makelist(list_t* argl);

#endif
