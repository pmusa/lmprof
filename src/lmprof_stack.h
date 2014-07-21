#ifndef LMPROF_STACK_H_INCLUDED
#define LMPROF_STACK_H_INCLUDED
 
#define LMPROF_STACK_SIZE 200

typedef struct lmprof_Stack lmprof_Stack;

lmprof_Stack* lmprof_stack_create    (void);
void          lmprof_stack_destroy   (lmprof_Stack* s);
int           lmprof_stack_push      (lmprof_Stack *s, size_t element);
int           lmprof_stack_equal     (lmprof_Stack *s, size_t nmem);
int           lmprof_stack_pop       (lmprof_Stack *s);
size_t        lmprof_stack_smart_pop (lmprof_Stack *s, size_t nmem);
 
#endif
