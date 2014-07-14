#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lmprof_hash.h"

#define hashpointer(ptr)        (((uintptr_t) ptr) % LMPROF_HASH_SIZE)

typedef struct lmprof_parent_hash {
  uintptr_t key;
  unsigned int count;
  size_t size;
  struct lmprof_parent_hash *next;
} lmprof_phash;

typedef struct lmprof_function_hash {
  uintptr_t key;
  char *name;
  unsigned int count;
  size_t size;
  lmprof_phash *parents[LMPROF_HASH_SIZE];
  struct lmprof_function_hash *next;
} lmprof_fhash;

struct lmprof_hash {
  unsigned int count;
  size_t size;
  lmprof_fhash *vfh[LMPROF_HASH_SIZE];
};

/* EXPORTED FUNCTIONS */
lmprof_hash* lmprof_hash_create  ();
void         lmprof_hash_destroy (lmprof_hash *h);
void         lmprof_hash_insert  (lmprof_hash *h, uintptr_t key,
                                                  const char *name);
void         lmprof_hash_update  (lmprof_hash *h, uintptr_t key,
                                  uintptr_t parent, size_t size);
int          lmprof_hash_exists  (lmprof_hash *h, uintptr_t key);


/* LOCAL FUNCTIONS */
static lmprof_phash* lmprof_phash_get(lmprof_phash **vph, uintptr_t key);
static lmprof_fhash* lmprof_fhash_get(lmprof_fhash **vph, uintptr_t key);
static void          lmprof_phash_upsert(lmprof_phash **vph, uintptr_t key,
                                                             size_t size);
static void          lmprof_phash_insert(lmprof_phash **vph, uintptr_t key,
                                                             lmprof_phash *val);
static void          lmprof_fhash_insert(lmprof_fhash **vfh, uintptr_t key,
                                                             lmprof_fhash *val);

/**
 *
 *
 *
 **/
static lmprof_phash* lmprof_phash_get(lmprof_phash **vph, uintptr_t key) {
  int index = hashpointer(key);
  lmprof_phash *ph = vph[index];
  if (ph != NULL) {
    while(ph != NULL) {
      if (key == ph->key) {
        return ph;
      }
      ph = ph->next;
    }
  }
  return ph;
}

static lmprof_fhash* lmprof_fhash_get(lmprof_fhash **vfh, uintptr_t key) {
  int index = hashpointer(key);
  lmprof_fhash *fh = vfh[index];
  if (fh != NULL) {
    while(fh != NULL) {
      if (key == fh->key) {
        return fh;
      }
      fh = fh->next;
    }
  }
  return fh;
}


static void lmprof_phash_insert(lmprof_phash **vph, uintptr_t key, 
                                                    lmprof_phash *val) {
  int index = hashpointer(key);
  lmprof_phash *ph = vph[index];
  val->next = ph;
  vph[index] = val;

/*
  if (ph == NULL) {
    vph[index] = val;
  } else {
    while(ph->next != NULL) {
      ph = ph->next;
    }
    ph->next = val;
  }
*/
}

static void lmprof_fhash_insert(lmprof_fhash **vfh, uintptr_t key,
                                                    lmprof_fhash *val) {
  int index = hashpointer(key);
  lmprof_fhash *fh = vfh[index];
  val->next = fh; 
  vfh[index] = val;

/*
  if (fh == NULL) {
    vfh[index] = val;
  } else {
    while(fh->next != NULL) {
      fh = fh->next;
    }
    fh->next = val;
  }
*/
}

static void lmprof_phash_upsert(lmprof_phash **vph, uintptr_t key, size_t size){
  lmprof_phash *val = lmprof_phash_get(vph, key);
  if (val != NULL) {
    val->count = val->count + 1;
    val->size  = val->size + size;
  } else {
    val = (lmprof_phash *) malloc (sizeof(lmprof_phash));
    val->key = key;
    val->count = 1;
    val->size = size;
    val->next = NULL;
    lmprof_phash_insert(vph, key, val);
  }
}


/* GLOBAL FUNCTIONS */

int lmprof_hash_exists(lmprof_hash *hash, uintptr_t key) {
  return lmprof_fhash_get(hash->vfh, key) != NULL;
}

void lmprof_hash_insert(lmprof_hash *hash, uintptr_t key,
                                          const char *name) {
  int i;
  lmprof_fhash *val = (lmprof_fhash *) malloc (sizeof(lmprof_fhash));
  val->key = key;
  val->name = (char *) malloc (strlen(name) + 1);
  strcpy(val->name, name);
  val->count = 0;
  val->size = 0;
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    val->parents[i] = NULL;
  }
  val->next = NULL;
  lmprof_fhash_insert(hash->vfh, key, val);
}

void lmprof_hash_update(lmprof_hash *hash, uintptr_t key, uintptr_t parent, size_t size) {
  lmprof_fhash *fh;
  hash->count = hash->count + 1;
  hash->size  = hash->size + size;

  fh = lmprof_fhash_get(hash->vfh, key);
  if (fh != NULL) {
    fh->count = fh->count + 1;
    fh->size = fh->size + size;
    lmprof_phash_upsert(fh->parents, parent, size);
  }
}

lmprof_hash* lmprof_hash_create() {
  int i;
  lmprof_hash *hash = (lmprof_hash *) malloc (sizeof(lmprof_hash));
  hash->count = 0;
  hash->size  = 0;
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    hash->vfh[i] = NULL;
  }
  return hash;
}

void lmprof_hash_destroy(lmprof_hash *hash) {
  int i;
  for(i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_fhash *fh;
    lmprof_fhash *vfhead = hash->vfh[i];
    while (vfhead != NULL) {
      int k;
      fh = vfhead;
      for (k = 0; k < LMPROF_HASH_SIZE; k++) {
        lmprof_phash *ph;
        lmprof_phash *vphead = fh->parents[k]; 
        while(vphead != NULL) {
          ph = vphead;
          vphead = vphead->next;
          free(ph);
        }
      }
      free(fh->name);
      vfhead = vfhead->next;
      free(fh);
    }
  }
  free(hash);
}

void lmprof_hash_print(lmprof_hash *hash, const char *file_name) {
  int i, k;
  FILE *f = fopen(file_name, "w");
  if (f == NULL) {
    printf("error: could open file: '%s'.", file_name);
    exit(1);
  }

  fprintf(f, "return {\n");
  fprintf(f, "  count = %d,\n", hash->count);
  fprintf(f, "  size = %lu,\n", hash->size);
  for (i = 0; i < LMPROF_HASH_SIZE; i++) {
    lmprof_fhash *fh = hash->vfh[i];
    while (fh != NULL) {
      fprintf(f, "  [%d] = {\n", (int) fh->key);
      fprintf(f, "    name = '%s',\n", fh->name);
      fprintf(f, "    count = %d,\n", fh->count);
      fprintf(f, "    size = %lu,\n", fh->size);
      fprintf(f, "    parents = {\n");
      for (k = 0; k < LMPROF_HASH_SIZE; k++) {
        lmprof_phash *ph = fh->parents[k];
        while (ph != NULL) {
          fprintf(f, "      [%d] = {\n", (int) ph->key);
          fprintf(f, "        count = %d,\n", ph->count);
          fprintf(f, "        size = %lu,\n", ph->size);
          fprintf(f, "      },\n");
          ph = ph->next;
        }
      }
      fprintf(f, "    },\n");
      fprintf(f, "  },\n");
      fh = fh->next;
    }
  }
  fprintf(f, "}\n");

  fclose(f);
}
