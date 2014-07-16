#include <stdlib.h>
#include <stdio.h>

#include "lmprof_stack.h"

struct lmprof_stack {
  size_t stack[LMPROF_STACK_SIZE];
  size_t fix[LMPROF_STACK_SIZE];
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

int lmprof_stack_push(lmprof_stack *s, size_t e) {
  s->top++;
  if (s->top >= LMPROF_STACK_SIZE) {
    return 1;
  }
  s->stack[s->top] = e;
  s->fix[s->top] = 0;
  return 0;
}

int lmprof_stack_is_diff(lmprof_stack *s, size_t nmem) {
  if (s->top > -1) {
    return nmem != s->stack[s->top];
  }
  return 1;
}
 
size_t lmprof_stack_pop(lmprof_stack *s) {
  s->top--;
  if (s->top < -1) {
    printf("error: Unable to pop from empty stack.\n");
    exit(1);
  }
  return s->stack[s->top + 1];
}

size_t lmprof_stack_smart_pop(lmprof_stack *s, size_t nmem) {
  size_t dmem;

  s->top--;
  if (s->top < -1) {
    printf("error: Unable to pop from empty stack.\n");
    exit(1);
  }

  dmem = nmem - s->stack[s->top + 1];
  if (s->top != 0) {  /* not the last element */
    /* increment fix by the present diff */
    s->fix[s->top] = s->fix[s->top] + dmem;
  }

  /* memory diff - fix value */
  return dmem - s->fix[s->top + 1];
}

