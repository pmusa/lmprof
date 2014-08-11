#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lmprof_hash.h"

#define hashpointer(fptr, pptr)  ((((uintptr_t) fptr) ^ ((uintptr_t) pptr)) \
                                                        % LMPROF_HASH_SIZE)
/*
** {==================================================================
** STRUCTS
** ===================================================================
*/

typedef struct lmprof_FHash {
  uintptr_t function;
  uintptr_t parent;
  char *name;
  unsigned int count;
  size_t size;
  struct lmprof_FHash *next;
} lmprof_FHash;

/* }================================================================== */

/*
** {==================================================================
** LOCAL FUNCTIONS
** ===================================================================
*/

/* insert val into hash using a combination of function and parent as hash. */
static void fhash_insert (lmprof_Hash *h, uintptr_t function,
                                          uintptr_t parent, lmprof_FHash *val) {
  int index = hashpointer(function, parent);
  lmprof_FHash *fh = h[index];
  val->next = fh;
  h[index] = val;
}

/* }================================================================== */

/*
** {==================================================================
** GLOBAL FUNCTIONS
** ===================================================================
*/

lmprof_Hash lmprof_hash_get (lmprof_Hash *h, uintptr_t function,
                                             uintptr_t parent) {
  int index = hashpointer(function, parent);
  lmprof_Hash fh = h[index];
  while(fh != NULL) {
    if (function == fh->function && parent == fh->parent) {
      return fh;
    }
    fh = fh->next;
  }
  return fh;
}

lmprof_Hash lmprof_hash_insert (lmprof_Hash *h, uintptr_t function,
                                uintptr_t parent, const char *name) {
  lmprof_FHash *val = (lmprof_FHash *) malloc (sizeof(lmprof_FHash));
  val->function = function;
  val->parent = parent;
  val->name = (char *) malloc (strlen(name) + 1);
  strcpy(val->name, name);
  val->count = 0;
  val->size = 0;
  /* val->next = NULL; inserting in front, no need for that */
  fhash_insert(h, function, parent, val);
  return val;
}

void lmprof_hash_update (lmprof_Hash *hash, lmprof_Hash v, size_t size) {
  if (v != NULL) {
    if (size < 0) { /*  TODO remove for production */
      printf("ERROR NEGATIVE SIZE IN UPDATE %lu - %lu\n", v->size, size);
      exit(1);
    }
    v->count = v->count + 1;
    v->size = v->size + size;
  }
}

lmprof_Hash* lmprof_hash_create (void) {
  lmprof_Hash* h = (lmprof_Hash*) malloc (LMPROF_HASH_SIZE * sizeof(lmprof_Hash));
  int i;
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    h[i] = NULL;
  }
  return h;
}

void lmprof_hash_destroy (lmprof_Hash *h) {
  int i;
  for(i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_FHash *fh;
    lmprof_FHash *vfhead = h[i];
    while (vfhead != NULL) {
      fh = vfhead;
      free(fh->name);
      vfhead = vfhead->next;
      free(fh);
    }
  }
  free(h);
}

void lmprof_hash_print (lmprof_Hash *h, const char *filename) {
  int i;
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    printf("error: could open file: '%s'.", filename);
    exit(1);
  }

  fprintf(f, "return {\n");
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_FHash *fh = h[i];
    while (fh != NULL) {
      fprintf(f, "  [\"%lu%lu\"] = {\n", fh->function, fh->parent);
      fprintf(f, "    func = \"%lu\",\n", fh->function);
      fprintf(f, "    parent = \"%lu\",\n", fh->parent);
      fprintf(f, "    name = \"%s\",\n", fh->name);
      fprintf(f, "    calls = %d,\n", fh->count);
      fprintf(f, "    mem_self = %lu,\n", fh->size);
      fprintf(f, "  },\n");
      fh = fh->next;
    }
  }
  fprintf(f, "}\n");

  fclose(f);
}

/* }================================================================== */

