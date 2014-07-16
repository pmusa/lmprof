#ifndef LMPROF_STACK_H_INCLUDED
#define LMPROF_STACK_H_INCLUDED
 
#define LMPROF_STACK_SIZE 200

typedef struct lmprof_stack lmprof_stack;

lmprof_stack* lmprof_stack_create  ();
void          lmprof_stack_destroy (lmprof_stack* s);
int           lmprof_stack_push    (lmprof_stack *s, size_t element);
size_t        lmprof_stack_pop     (lmprof_stack *s);
 
#endif
