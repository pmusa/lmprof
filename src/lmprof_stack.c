#include <stdlib.h>
#include <stdio.h>

#include "lmprof_stack.h"

struct lmprof_Stack {
  size_t stack[LMPROF_STACK_SIZE];
  size_t fix[LMPROF_STACK_SIZE];
  int top;
};

lmprof_Stack* lmprof_stack_create (void) {
  lmprof_Stack* s = (lmprof_Stack*) malloc (sizeof(lmprof_Stack));
  s->top = -1;
  return s;
}

void lmprof_stack_destroy (lmprof_Stack *s) {
  free(s);
}

int lmprof_stack_push (lmprof_Stack *s, size_t e) {
  s->top++;
  if (s->top >= LMPROF_STACK_SIZE) {
    return 1;
  }
  s->stack[s->top] = e;
  s->fix[s->top] = 0;
  return 0;
}

int lmprof_stack_equal (lmprof_Stack *s, size_t nmem) {
  if (s->top > -1) {
    return nmem == s->stack[s->top];
  }
  return 1;  /* better for error handling */
}
 
int lmprof_stack_pop (lmprof_Stack *s) {
  s->top--;
  if (s->top < -1) {
    return 1;
  }
  return 0;
}

/* ATTENTION, this function must should never be called with an empty stack */
size_t lmprof_stack_smart_pop (lmprof_Stack *s, size_t nmem, size_t *tot_mem) {
  s->top--;

  /* not cheking empty stack, performance purpose */

  *tot_mem = nmem - s->stack[s->top + 1];
  if (s->top >= 0) {
    /* increment fix by the present diff */
    s->fix[s->top] = s->fix[s->top] + *tot_mem;
  }

  /* memory diff - fix value */
  return *tot_mem - s->fix[s->top + 1];
}

