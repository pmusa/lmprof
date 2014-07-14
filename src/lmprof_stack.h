#ifndef LMPROF_STACK_H_INCLUDED
#define LMPROF_STACK_H_INCLUDED
 
#define LMPROF_STACK_SIZE 20

typedef struct lmprof_stack lmprof_stack;

lmprof_stack* lmprof_stack_create  ();
void          lmprof_stack_destroy (lmprof_stack* s);
int           lmprof_stack_push    (lmprof_stack *s, int element);
int           lmprof_stack_pop     (lmprof_stack *s);
 
#endif
