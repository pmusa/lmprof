#ifndef LMPROF_HASH_H_INCLUDED
#define LMPROF_HASH_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
 
#define LMPROF_HASH_SIZE 17

typedef struct lmprof_fhash* lmprof_hash;

lmprof_hash* lmprof_hash_create  ();
void         lmprof_hash_destroy (lmprof_hash *h);
lmprof_hash  lmprof_hash_insert  (lmprof_hash *h, uintptr_t function,
                                  uintptr_t parent, const char *name);
void         lmprof_hash_update  (lmprof_hash *h, lmprof_hash v, size_t size);
lmprof_hash  lmprof_hash_get     (lmprof_hash *h, uintptr_t function,
                                                  uintptr_t parent);
void         lmprof_hash_print   (lmprof_hash *h, const char *file_name);

#endif
