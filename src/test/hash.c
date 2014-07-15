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
    uintptr_t function = rand() % 10;
    uintptr_t parent = rand() % 10;
    size_t size = rand() % 100;
    lmprof_hash v = lmprof_hash_get(h, function, parent);
    if (v == NULL) {
      char name[3];
      name[0] = 'f'; name[1] = '0' + function; name[2] = '\0';
//printf("Inserting k: %d - n: %s\n", (int) key, name);
      v = lmprof_hash_insert(h, function, parent, name);
    } 
printf("Updating f: %d - p: %d - size: %d\n", (int)function, (int)parent, (int)size);
    lmprof_hash_update(h, v, size);
  }

  lmprof_hash_print(h, "hash_test.lua");
  lmprof_hash_destroy(h);
//printf("Hash destroyed\n");

  return 0;
}

