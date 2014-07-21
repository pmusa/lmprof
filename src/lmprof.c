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
#include "lmprof_lstrace.h"

#define LMPROF_DEFAULT_OUTPUT_FILENAME	"lmprof_default_output.txt"
#define LMPROF_STACK_DUMP_FILENAME	"lmprof_lstrace.dump"
#define LMPROF_UD_ALLOC			"lmprof_alloc"
#define LMPROF_GET_STATE_INFO(L)	&gst
#define TRUE				1
#define FALSE				0


/*
** {==================================================================
** STRUCTS
** ===================================================================
*/

/* Keeps the default allocation function and the ud of a lua_State */
typedef struct lmprof_Alloc {
  lua_Alloc f;
  void *ud;
} lmprof_Alloc;

/* Structure that keeps allocation info about the running State */
typedef struct lmprof_State {
  /* stack containing memory use when entering a function */
  lmprof_Stack *mem_stack;
  /* hash table containning information of each function call that generated
   * some allocation. */
  lmprof_Hash  *func_calls;
  /* flag used to avoid first return after setting the hook */
  int ignore_return;
  /* memory counter for each allocation */
  size_t alloc_count;
} lmprof_State;

/* }================================================================== */

/*
** {==================================================================
** FUNCTIONS DECLARATION
** ===================================================================
*/

/* FUNCTIONS REGISTERED TO BE USED */
static int start  (lua_State *L);
static int stop   (lua_State *L);
static int pause  (lua_State *L);
static int resume (lua_State *L);
static int write  (lua_State *L);

/* LOCAL FUNCTIONS */
static void destroy(lua_State *L, lmprof_State *st, const char *filename);
static void create_finalizer(lua_State *L, lua_Alloc f, void *ud);
static void update(lua_State *L, lmprof_State *st, lua_Debug * far, size_t mem);

/* FUNCTIONS USED BY LUA */

/* allocation function used by Lua when luamemprofiler is used */
static int   finalize (lua_State *L);
static void* alloc    (void *ud, void *ptr, size_t osize, size_t nsize);
static void  hook     (lua_State *L, lua_Debug *ar);

/* }================================================================== */

/*
** {==================================================================
** STATIC VARIABLES
** ===================================================================
*/
static lmprof_State gst;

/* }================================================================== */

/*
** {==================================================================
** FUNCTIONS IMPLEMENTATION
** ===================================================================
*/

static void update (lua_State *L, lmprof_State *st, lua_Debug * far, size_t mem) {
  lmprof_Hash v;
  uintptr_t function;
  uintptr_t parent;
  lua_Debug par;

  lua_getinfo(L, "f", far);
  function = (uintptr_t) lua_topointer(L, -1);

  lua_getstack(L, 1, &par);
  lua_getinfo(L, "f", &par);
  parent = (uintptr_t) lua_topointer(L, -1);

  v = lmprof_hash_get(st->func_calls, function, parent);
  if (v == NULL) {
    const char *name = lmprof_lstrace_getfuncinfo(L, far);
    v = lmprof_hash_insert(st->func_calls, function, parent, name);
  }

  lmprof_hash_update(st->func_calls, v, mem);
}


static void set_lua_alloc (lua_State *L, int remove_finalizer) {
  lmprof_Alloc *s;

  /* get original alloc function and opaque pointer */
  lua_pushstring(L, LMPROF_UD_ALLOC);
  lua_rawget(L, LUA_REGISTRYINDEX);
  s = (lmprof_Alloc*) lua_touserdata(L, -1);

  /* the library was not initialized (start not called) */
  if (s == NULL) {
    luaL_error(L, "lmprof was not properly innitialized. Try calling 'start'.");
  }

  /* restore original allocation function */
  lua_setallocf(L, s->f, s->ud);

  if (remove_finalizer) {
    lua_pushnil(L);
    lua_setmetatable(L, -2);
  }
}

static void set_lmprof_alloc (lua_State *L, int insert_finalizer) {
  static lua_Alloc f;
  static void *ud;
  /* get default allocation function */
  f = lua_getallocf(L, &ud);

  /* check if start has been called before */
  if (f == alloc) {
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
  if (insert_finalizer) {
    create_finalizer(L, f, ud);
    lua_setallocf(L, alloc, ud);
  }
}


static void hook (lua_State *L, lua_Debug *ar) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  if (st->ignore_return) {
    st->ignore_return = 0;
    return;
  }

  switch (ar->event) {
    case LUA_HOOKCALL: {
      int err = lmprof_stack_push(st->mem_stack, st->alloc_count);
      if (err) {
        lmprof_lstrace_write(L, LMPROF_STACK_DUMP_FILENAME);
        set_lua_alloc(L, TRUE);
        destroy(L, st, LMPROF_DEFAULT_OUTPUT_FILENAME);
        luaL_error(L, "lmprof stack overflow (current size = %d). We consider \
your call chain very big. '%s' contains the full stack trace and '%s' contais \
the memory profile.", LMPROF_STACK_SIZE, LMPROF_STACK_DUMP_FILENAME,
                                         LMPROF_DEFAULT_OUTPUT_FILENAME);
      }
      break;
    }
    case LUA_HOOKRET:
      if (lmprof_stack_equal(st->mem_stack, st->alloc_count)) {
        int err = lmprof_stack_pop(st->mem_stack);
        if (err) {
          set_lua_alloc(L, TRUE);
          destroy(L, st, LMPROF_DEFAULT_OUTPUT_FILENAME);
          luaL_error(L, "unable to pop from empty stack. you did not call stop \
or called stop in a level 'upper' than start was called.");
        }
      } else {
        size_t mem = lmprof_stack_smart_pop(st->mem_stack, st->alloc_count);
        update(L, st, ar, mem);
      }
      /* TODO: tailcall case, there is a BUG */
      break;
  }
}

static void destroy (lua_State *L, lmprof_State *st, const char *filename) {
  lmprof_stack_destroy(st->mem_stack);
  lmprof_hash_print(st->func_calls, filename);
  lmprof_hash_destroy(st->func_calls);
  lua_sethook(L, hook, 0, 0);
}

/*
** Called when main program ends.
** Restores lua_State original allocation function.
*/
static int finalize (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
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
    destroy(L, st, LMPROF_DEFAULT_OUTPUT_FILENAME);
  }

  return 0;
}

/* Register finalize function as metatable */
static void create_finalizer (lua_State *L, lua_Alloc f, void *ud) {
  lmprof_Alloc *s;

  /* create metatable with finalize function (__gc field) */
  luaL_newmetatable(L, "lmprof_mt");
  lua_pushcfunction(L, finalize);
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


static void *alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  lmprof_State *st = &gst;  /* TODO: solve this problem */
  (void) ud; /* not used */

  if (nsize == 0) {  /* check lua manual for more details */
    free(ptr);
    return NULL;
  } else if (ptr == NULL) {  /* case (malloc): osize holds object type */
    osize = 0;
  }
  st->alloc_count = st->alloc_count + (nsize - osize);
  return realloc(ptr, nsize);
}

static int start (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  lua_Debug ar;

  set_lmprof_alloc(L, TRUE);

  /* create auxiliary structures and set call/return hook */
  st->alloc_count = 0;
  st->ignore_return = 1;
  st->mem_stack = lmprof_stack_create();
  st->func_calls = lmprof_hash_create();

  /* register calling function into hash */
  lua_getstack(L, 1, &ar);
  update(L, st, &ar, 0);

  lua_sethook(L, hook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 0;
}

static int stop (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  const char *filename = LMPROF_DEFAULT_OUTPUT_FILENAME;

  if (lua_gettop(L)) {
    filename = luaL_checkstring(L, 1);
  }

  set_lua_alloc(L, TRUE);
  destroy(L, st, filename);
  return 0;
}

static int pause (lua_State *L) {
  set_lua_alloc(L, FALSE);
  lua_sethook(L, hook, 0, 0);
  return 0;
}

static int resume (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  set_lmprof_alloc(L, FALSE);
  st->ignore_return = 1;
  lua_sethook(L, hook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 0;
}

static int write (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  lmprof_hash_print(st->func_calls, filename);
  return 0;
}

/* luamemprofiler function registration array */
static const luaL_Reg lmprof[] = {
  { "start",  start},
  { "stop",   stop},
  { "pause",  pause},
  { "resume", resume},
  { "write",  write},
  { NULL, NULL }
};

/* register luamemprofiler functions */
LUALIB_API int luaopen_lmprof (lua_State *L) {
  luaL_newlib(L, lmprof);
  return 1;
}

/* }================================================================== */

