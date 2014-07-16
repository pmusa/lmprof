#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lmprof_hash.h"

#define hashpointer(fptr, pptr)  ((((uintptr_t) fptr) ^ ((uintptr_t) pptr)) \
                                                        % LMPROF_HASH_SIZE)

typedef struct lmprof_fhash {
  uintptr_t function;
  uintptr_t parent;
  char *name;
  unsigned int count;
  size_t size;
  struct lmprof_fhash *next;
} lmprof_fhash;

static lmprof_fhash* h[LMPROF_HASH_SIZE];

/* EXPORTED FUNCTIONS */
lmprof_hash* lmprof_hash_create  ();
void         lmprof_hash_destroy (lmprof_hash *h);
lmprof_hash  lmprof_hash_insert  (lmprof_hash *h, uintptr_t function,
                                  uintptr_t parent, const char *name);
void         lmprof_hash_update  (lmprof_hash *h, lmprof_hash v, size_t size);
lmprof_hash  lmprof_hash_get     (lmprof_hash *h, uintptr_t function,
                                                  uintptr_t parent);
void         lmprof_hash_print   (lmprof_hash *h, const char *file_name);

/* LOCAL FUNCTIONS */

/**
 *
 * insert val into hash using a combination of function and parent as hash.
 *
 **/
static void lmprof_fhash_insert(lmprof_hash *h, uintptr_t function,
                                uintptr_t parent, lmprof_fhash *val) {
  int index = hashpointer(function, parent);
  lmprof_fhash *fh = h[index];
  val->next = fh;
  h[index] = val;
}

/* GLOBAL FUNCTIONS */

lmprof_hash lmprof_hash_get(lmprof_hash *h, uintptr_t function,
                                            uintptr_t parent) {
  int index = hashpointer(function, parent);
  lmprof_hash fh = h[index];
  while(fh != NULL) {
    if (function == fh->function && parent == fh->parent) {
      return fh;
    }
    fh = fh->next;
  }
  return fh;
}

lmprof_hash lmprof_hash_insert(lmprof_hash *h, uintptr_t function,
                               uintptr_t parent, const char *name) {
  lmprof_fhash *val = (lmprof_fhash *) malloc (sizeof(lmprof_fhash));
  val->function = function;
  val->parent = parent;
  val->name = (char *) malloc (strlen(name) + 1);
  strcpy(val->name, name);
  val->count = 0;
  val->size = 0;
  /* val->next = NULL; inserting in front, no need for that */
  lmprof_fhash_insert(h, function, parent, val);
  return val;
}

void lmprof_hash_update(lmprof_hash *hash, lmprof_hash v, size_t size) {
  if (v != NULL) {
    v->count = v->count + 1;
    v->size = v->size + size;
  }
}

lmprof_hash* lmprof_hash_create() {
  int i;
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    h[i] = NULL;
  }
  return h;
}

void lmprof_hash_destroy(lmprof_hash *h) {
  int i;
  for(i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_fhash *fh;
    lmprof_fhash *vfhead = h[i];
    while (vfhead != NULL) {
      fh = vfhead;
      free(fh->name);
      vfhead = vfhead->next;
      free(fh);
    }
  }
  return;
}

void lmprof_hash_print(lmprof_hash *h, const char *file_name) {
  int i;
  FILE *f = fopen(file_name, "w");
  if (f == NULL) {
    printf("error: could open file: '%s'.", file_name);
    exit(1);
  }

  fprintf(f, "return {\n");
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_fhash *fh = h[i];
    while (fh != NULL) {
      fprintf(f, "  [\"%d_%d\"] = {\n", (int) fh->function, (int) fh->parent);
      fprintf(f, "    func = '%d',\n", (int) fh->function);
      fprintf(f, "    parent = '%d',\n", (int) fh->parent);
      fprintf(f, "    name = '%s',\n", fh->name);
      fprintf(f, "    count = %d,\n", fh->count);
      fprintf(f, "    size = %lu,\n", fh->size);
      fprintf(f, "  },\n");
      fh = fh->next;
    }
  }
  fprintf(f, "}\n");

  fclose(f);
}
