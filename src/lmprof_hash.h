#ifndef LMPROF_HASH_H_INCLUDED
#define LMPROF_HASH_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
 
#define LMPROF_HASH_SIZE 17

typedef struct lmprof_FHash* lmprof_Hash;

lmprof_Hash* lmprof_hash_create  (void);
void         lmprof_hash_destroy (lmprof_Hash *h);
lmprof_Hash  lmprof_hash_insert  (lmprof_Hash *h, uintptr_t function,
                                  uintptr_t parent, const char *name);
void         lmprof_hash_update  (lmprof_Hash *h, lmprof_Hash v, size_t size);
lmprof_Hash  lmprof_hash_get     (lmprof_Hash *h, uintptr_t function,
                                                  uintptr_t parent);
void         lmprof_hash_print   (lmprof_Hash *h, const char *filename);

#endif
