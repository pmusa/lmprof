#ifndef LMPROF_HASH_H_INCLUDED
#define LMPROF_HASH_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
 
#define LMPROF_HASH_SIZE 17

typedef struct lmprof_hash lmprof_hash;

lmprof_hash* lmprof_hash_create  ();
void         lmprof_hash_destroy (lmprof_hash *h);
void         lmprof_hash_insert  (lmprof_hash *h, uintptr_t key,
                                                  const char *name);
void         lmprof_hash_update  (lmprof_hash *h, uintptr_t key,
                                  uintptr_t parent, size_t size);
int          lmprof_hash_exists  (lmprof_hash *h, uintptr_t key);
void         lmprof_hash_print   (lmprof_hash *h, const char *file_name);


#endif
