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
  size_t self_size;
  size_t cum_size;
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
  int i, namelen;

  lmprof_FHash *val = (lmprof_FHash *) malloc (sizeof(lmprof_FHash));
  val->function = function;
  val->parent = parent;
  val->count = 0;
  val->self_size = 0;
  val->cum_size = 0;

  /* copy string name from Lua removing dangerous (" --) char */
  /* multi-state beginning with more than two -- will have problems */
  namelen = strlen(name);
  val->name = (char *) malloc (namelen + 1);
  for(i=0; i < namelen; i++) {
    if (name[i] == '"') {
      val->name[i] = '\'';
    } else if (name[i] == '-' && name[i+1] == '-') {
      val->name[i] = ' ';
    } else {
      val->name[i] = name[i];
    }
  }
  val->name[i] = '\0';

  /* val->next = NULL; inserting in front, no need for that */
  fhash_insert(h, function, parent, val);
  return val;
}

void lmprof_hash_update (lmprof_Hash *hash, lmprof_Hash v, size_t self_size,
                                                           size_t cum_size) {
  if (v != NULL) {
    v->count = v->count + 1;
    v->self_size = v->self_size + self_size;
    v->cum_size = v->cum_size + cum_size;
  }
}

lmprof_Hash* lmprof_hash_create (void) {
  lmprof_Hash* h = (lmprof_Hash*) malloc (LMPROF_HASH_SIZE*sizeof(lmprof_Hash));
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
      fprintf(f, "    mem_self = %lu,\n", fh->self_size);
      fprintf(f, "    mem_cum = %lu,\n", fh->cum_size);
      fprintf(f, "  },\n");
      fh = fh->next;
    }
  }
  fprintf(f, "}\n");

  fclose(f);
}

/* }================================================================== */

