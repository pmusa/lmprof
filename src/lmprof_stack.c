#include <stdlib.h>
#include <stdio.h>

#include "lmprof_stack.h"

struct lmprof_stack {
  int stack[LMPROF_STACK_SIZE];
  int top;
};

lmprof_stack* lmprof_stack_create() {
  lmprof_stack* s = (lmprof_stack*) malloc (sizeof(lmprof_stack));
  s->top = -1;
  return s;
}

void lmprof_stack_destroy(lmprof_stack *s) {
  free(s);
}

int lmprof_stack_push(lmprof_stack *s, int e) {
  s->top++;
  if (s->top > LMPROF_STACK_SIZE) {
    return 1;
  }
  s->stack[s->top] = e;
  return 0;
}

int lmprof_stack_pop(lmprof_stack *s) {
  s->top--;
  if (s->top < -1) {
    printf("error: Unable to pop from empty stack.\n");
    exit(1);
  }
  return s->stack[s->top + 1];
}
 
