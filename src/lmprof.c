/*
** Author: Pablo Musa
** Creation Date: may 19 2014
** Last Modification: ago 15 2014
** See Copyright Notice in COPYRIGHT
**
*/

#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lmprof_stack.h"
#include "lmprof_hash.h"

#define LMPROF_FILENAME		"lmprof_default_output.txt"
#define LMPROF_UD_ALLOC		"lmprof_alloc"

/*****************************************************************************
 * STRUCTS  ******************************************************************
 *****************************************************************************/

/* Keeps the default allocation function and the ud of a lua_State */
typedef struct lmprof_Alloc {
  lua_Alloc f;
  void *ud;
} lmprof_Alloc;

/*****************************************************************************
 * FUNCTIONS DECLARATION *****************************************************
 *****************************************************************************/

/* FUNCTIONS REGISTERED TO BE USED */
static int lmprof_start  (lua_State *L);
static int lmprof_stop   (lua_State *L);
static int lmprof_pause  (lua_State *L);
static int lmprof_resume (lua_State *L);
static int lmprof_write  (lua_State *L);

/* LOCAL FUNCTIONS */
static void lmprof_destroy(lua_State *L, const char *filename);
static void lmprof_create_finalizer(lua_State *L, lua_Alloc f, void *ud);
static void lmprof_update(lua_State *L, lua_Debug * far, int omem, int nmem);

/* FUNCTIONS USED BY LUA */
/* allocation function used by Lua when luamemprofiler is used */
int lmprof_finalize (lua_State *L);
void *lmprof_alloc (void *ud, void *ptr, size_t osize, size_t nsize);
void lmprof_hook(lua_State *L, lua_Debug *ar);


/*****************************************************************************
 * STATIC VARIABLES **********************************************************
 *****************************************************************************/
/*
 * stack containing memory use when entering a function
 */
static lmprof_stack *mem_stack;

/*
 * hash table containning information of each function call that generated
 * some allocation.
 */
static lmprof_hash  *func_calls;

/*
 * flag used to avoid first return after setting the hook
 */
static int ignore_first_return;

/*
 * memory counter for each allocation
 */
static size_t alloc_count;

/*****************************************************************************
 * FUNCTIONS IMPLEMENTATION **************************************************
 *****************************************************************************/

static lmprof_hash insert(lua_State *L, uintptr_t function, uintptr_t parent,
                                                     lua_Debug *ar) {
  const char *name;
  lua_getinfo(L, "n", ar);
  if (ar-> name != NULL) {
/*printf("name: %s - namewhat: %s\n", ar->name, ar->namewhat);*/
    name = ar->name;
  } else {
    lua_getinfo(L, "S", ar);
/*printf("source: %s - short_src: %s - linedefined: %d - lastlinedefined: %d - what: %s\n", ar->source, ar->short_src, ar->linedefined, ar->lastlinedefined, ar->what); */
    name = ar->what;
  }
  lua_settop(L, -1);
  return lmprof_hash_insert(func_calls, function, parent, name);
}

static void lmprof_update(lua_State *L, lua_Debug * far, int omem, int nmem) {
  lmprof_hash v;
  uintptr_t function;
  uintptr_t parent;
  lua_Debug par;

  lua_getinfo(L, "f", far);
  function = (uintptr_t) lua_topointer(L, -1);

  lua_getstack(L, 1, &par);
  lua_getinfo(L, "f", &par);
  parent = (uintptr_t) lua_topointer(L, -1);

  v = lmprof_hash_get(func_calls, function, parent);
  if ( v == NULL ) {
    v = insert(L, function, parent, far);
  }

  lmprof_hash_update(func_calls, v, nmem - omem);
  lua_settop(L, -2);
}

void lmprof_hook(lua_State *L, lua_Debug *ar) {
  if (ignore_first_return) {
    ignore_first_return = 0;
    return;
  }

  if (ar->event == LUA_HOOKCALL) {
    int nok = lmprof_stack_push(mem_stack, alloc_count);
    if (nok) {
      lmprof_hash_print(func_calls, LMPROF_FILENAME);
      lua_pushstring(L, "call stack is too big. Check for strange behaviour in your code");
      lua_error(L);
    }
  } else if (ar->event == LUA_HOOKRET) {
    size_t nmem = alloc_count;
    size_t omem = lmprof_stack_pop(mem_stack);
    if (nmem > omem) {
      lmprof_update(L, ar, omem, nmem);
      /*printf("Mudanca de Tamanho: %d - %d - %d\n", omem, nmem, nmem - omem);*/
    } else if (omem < nmem) {
      lua_pushstring(L, "bug in the library");
      lua_error(L);
    }
  }
}

static void lmprof_destroy(lua_State *L, const char *filename) {
  lmprof_stack_destroy(mem_stack);
  lmprof_hash_print(func_calls, filename);
  lmprof_hash_destroy(func_calls);
  lua_sethook (L, lmprof_hook, 0, 0);
}

/*
** Called when main program ends.
** Restores lua_State original allocation function.
*/
int lmprof_finalize (lua_State *L) {
  lmprof_Alloc *s;

  /* check lmprof_Alloc */
  if (!lua_isuserdata(L, -1)) {
    lua_pushstring(L, "incorrect argument");
    lua_error(L);
  }

  /* get lmprof_Alloc and restore original allocation function */
  s = (lmprof_Alloc *) lua_touserdata(L, -1);
  if (s->f != lua_getallocf (L, NULL)) {
    lua_setallocf(L, s->f, s->ud);
    lmprof_destroy(L, LMPROF_FILENAME);
  }

  return 0;
}

/* Register finalize function as metatable */
static void lmprof_create_finalizer(lua_State *L, lua_Alloc f, void *ud) {
  lmprof_Alloc *s;

  /* create metatable with finalize function (__gc field) */
  luaL_newmetatable(L, "lmprof_mt");
  lua_pushcfunction(L, lmprof_finalize);
  lua_setfield(L, -2, "__gc");

  /* create 'alloc' userdata (one ud for each Lua_State) */
  s = (lmprof_Alloc*) lua_newuserdata(L, (size_t) sizeof(lmprof_Alloc));
  s->f = f;
  s->ud = ud;

  /* set userdata metatable */
  luaL_setmetatable(L, "lmprof_mt");

  /* insert userdata into registry table so it cannot be collected */
  lua_setfield(L, LUA_REGISTRYINDEX, LMPROF_UD_ALLOC);
}


void *lmprof_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void) ud; /* not used */

  if (nsize == 0) {  /* check lua manual for more details */
    free(ptr);
    return NULL;
  } else if (ptr == NULL) {  /* case (malloc): osize holds object type */
    osize = 0;
  }
  alloc_count = alloc_count + (nsize - osize);
  return realloc(ptr, nsize);
}

static int lmprof_start(lua_State *L) {
  static lua_Alloc f;
  static void *ud;

  /* get default allocation function */
  f = lua_getallocf(L, &ud);

  /* check if start has been called before */
  if (f == lmprof_alloc) {
    /* restore default allocation function and remove library finalizer */
    lmprof_Alloc *s;
    lua_getfield(L, LUA_REGISTRYINDEX, LMPROF_UD_ALLOC);
    s = (lmprof_Alloc *) lua_touserdata(L, -1);
    lua_setallocf(L, s->f, s->ud);
    lua_getmetatable(L, -1);
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");

    lua_pushstring(L, "calling lmprof start function twice");
    lua_error(L);
  }

  /* create data_structure and set finalizer */
  lmprof_create_finalizer(L, f, ud);
  lua_setallocf(L, lmprof_alloc, ud);

  /* create auxiliary structures and set call/return hook */
  alloc_count = 0;
  ignore_first_return = 1;
  mem_stack = lmprof_stack_create();
  func_calls = lmprof_hash_create();
  lua_sethook(L, lmprof_hook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 0;
}

static int lmprof_stop(lua_State *L) {
  lmprof_Alloc *s;
  const char * filename = LMPROF_FILENAME;
  if (lua_gettop(L)) {
    filename = luaL_checkstring(L, 1);
  }

  /* get 'alloc' userdata and restore original allocation function */
  lua_pushstring(L, LMPROF_UD_ALLOC);
  lua_rawget(L, LUA_REGISTRYINDEX);
  s = (lmprof_Alloc*) lua_touserdata(L, -1);
  if (s == NULL) {
    lua_pushstring(L, "calling luamemprofiler stop function without calling start function");
    lua_error(L);
  }
  lua_pop(L, 1);
  lua_setallocf(L, s->f, s->ud);
  
  lmprof_destroy(L, filename);
  return 0;
}

static int lmprof_pause(lua_State *L) {
  lua_sethook(L, lmprof_hook, 0, 0);
  return 0;
}

static int lmprof_resume(lua_State *L) {
  ignore_first_return = 1;
  lua_sethook(L, lmprof_hook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 0;
}

static int lmprof_write(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lmprof_hash_print(func_calls, filename);
  return 0;
}

/* luamemprofiler function registration array */
static const luaL_Reg lmprof[] = {
  { "start",  lmprof_start},
  { "stop",   lmprof_stop},
  { "pause",  lmprof_pause},
  { "resume", lmprof_resume},
  { "write",  lmprof_write},
  { NULL, NULL }
};

/* register luamemprofiler functions */
LUALIB_API int luaopen_lmprof (lua_State *L) {
  luaL_newlib(L, lmprof);
  return 1;
}

