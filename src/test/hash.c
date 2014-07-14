#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "../lmprof_hash.h"

#define N 100

int main (void) {
  int i;
  lmprof_hash *h = lmprof_hash_create();

//printf("Hash created\n");
  for (i = 0; i < N; i++) {
    uintptr_t key = rand() % 10;
    uintptr_t parent = rand() % 10;
    size_t size = rand() % 100;
    if (!lmprof_hash_exists(h, key)) {
      char name[3];
      name[0] = 'f'; name[1] = '0' + key; name[2] = '\0';
//printf("Inserting k: %d - n: %s\n", (int) key, name);
      lmprof_hash_insert(h, key, name);
    } 
//printf("Updating k: %d - p: %d - size: %d\n", (int)key, (int)parent, (int)size);
    lmprof_hash_update(h, key, parent, size);
  }

  lmprof_hash_print(h, "hash_test.lua");
  lmprof_hash_destroy(h);
//printf("Hash destroyed\n");

  return 0;
}

