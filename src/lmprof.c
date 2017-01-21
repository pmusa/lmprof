#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

#include "lmprof_stack.h"
#include "lmprof_hash.h"
#include "lmprof_lstrace.h"

#define LMPROF_DEFAULT_OUTPUT_FILENAME	"lmprof_default_output.lua"
#define LMPROF_STACK_DUMP_FILENAME	"lmprof_lstrace.dump"
#define LMPROF_UD_ALLOC			"lmprof_alloc"
#define LMPROF_GET_STATE_INFO(L)	get_lmprof_state(L)
#define TRUE				1
#define FALSE				0
#define LMPROF_ROOT_FUNCTION_ID         0
#define LMPROF_UPDATE_ROOT              0
#define LMPROF_UPDATE_CALL              1
#define LMPROF_UPDATE_TAIL_CALL         2
#define LMPROF_SIZE_ZERO                0
#define LMPROF_STOP_QUEUE_SIZE		3


/*
** {==================================================================
** STRUCTS
** ===================================================================
*/
static long func_counter = 0;

/* Keeps the default allocation function and the ud of a lua_State */
typedef struct lmprof_Alloc {
  lua_Alloc f;
  void *ud;
} lmprof_Alloc;

/* Structure that keeps allocation info about the running State */
typedef struct lmprof_State {
  /* structure that keeps previous Lua allocation function and ud */
  lmprof_Alloc la;
  /* name of the output file */
  char *output_filename;
  /* stack containing memory use when entering a function */
  lmprof_Stack *mem_stack;
  /* hash table containning information of each function call that generated
   * some allocation. */
  lmprof_Hash  *func_calls;
  /* flag used to avoid first return after setting the hook */
  int ignore_return;
  /* memory counter for each allocation */
  size_t alloc_count;
  /* flag used to disable alloc_count increment inside lmprof calls */
  int increment_alloc_count;
  /* parent address of the tail call */
  uintptr_t tail_parent;
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


/* LOCAL FUNCTIONS */

/* cleans lmprof data and restore allocation function */
static void          destroy(lua_State *L, lmprof_State *st, int rm_finalizer);
/* updates memory allocation of a function */
static void          update(lua_State *L, lmprof_State *st, lua_Debug * far,
                            size_t mem, size_t total_mem, int update_types);
static lmprof_State* get_lmprof_state(lua_State *L);
static lmprof_State* new_lmprof_state(lua_State *L);

/* FUNCTIONS USED BY LUA */

/* restores previous Lua info and cleans lmprof data */
static int   finalize(lua_State *L);
/* allocation function used by Lua when lmprof is used */
static void* alloc(void *ud, void *ptr, size_t osize, size_t nsize);
/* handles Lua hook calls */
static void  hook(lua_State *L, lua_Debug *ar);

/* }================================================================== */

/*
** {==================================================================
** STATIC VARIABLES
** ===================================================================
*/

/* }================================================================== */

/*
** {==================================================================
** FUNCTIONS IMPLEMENTATION
** ===================================================================
*/

static lmprof_State* get_lmprof_state (lua_State *L) {
  lmprof_State *st;
  lua_getfield(L, LUA_REGISTRYINDEX, LMPROF_UD_ALLOC);
  st = (lmprof_State*) lua_touserdata(L, -1);
  lua_pop(L,1);
  return st;
}

static void update (lua_State *L, lmprof_State *st, lua_Debug * far,
                          size_t self_mem, size_t total_mem, int update_type) {
  lmprof_Hash v;
  uintptr_t function;
  uintptr_t parent;
  lua_Debug par;

  switch (update_type) {
    case LMPROF_UPDATE_ROOT:  /* stop was called, do not get parent */
      lua_getinfo(L, "f", far);
      function = (uintptr_t) lua_topointer(L, -1);
      parent = 0;
      break;
    case LMPROF_UPDATE_CALL:
      lua_getinfo(L, "f", far);
      function = (uintptr_t) lua_topointer(L, -1);
      if (lua_getstack(L, 1, &par)) {
        lua_getinfo(L, "f", &par);
        parent = (uintptr_t) lua_topointer(L, -1);
      } else {
        parent = 0;
      }
      break;
    case LMPROF_UPDATE_TAIL_CALL:
      /*
       * in tailcall cases, the current env is the new tailcall function
       * therefore, we need to first get the parent (returning function)
       * and then the grandparent (returning function parent)
       */
      lua_getstack(L, 1, far);
      lua_getinfo(L, "f", far);
      function = (uintptr_t) lua_topointer(L, -1);
      if (st->tail_parent != 0) {
        parent = st->tail_parent;
      } else {
        if (lua_getstack(L, 2, &par)) {
          lua_getinfo(L, "f", &par);
          parent = (uintptr_t) lua_topointer(L, -1);
        } else {
          parent = 0;
        }
      }
      st->tail_parent = function;
      break;
    default:
      luaL_error(L, "invalid update type. internal error");
      exit(1);
  }

  /* TODO: verify if parent == function and handle recursive calls 
     guardar f->f ou m->f?
     use st->rec
if (function == parent) {
  if (rec < 0) rec = 0;
  rec = rec + self_mem;
  return
}

if (rec > 0) {
  self_mem = self_mem + rec
  provavel gambiarra para atualizar o fix!!
}
}

  */

  v = lmprof_hash_get(st->func_calls, function, parent);
  if (v == NULL) {
    const char *name = lmprof_lstrace_getfuncinfo(L, far);
    v = lmprof_hash_insert(st->func_calls, function, parent, name);
  }

  lmprof_hash_update(st->func_calls, v, self_mem, total_mem);
}

static void hook (lua_State *L, lua_Debug *ar) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  st->increment_alloc_count = 0;  /* disable alloc count inside hook */
  if (st->ignore_return) {
    st->ignore_return = 0;
    return;
  }

func_counter++;

  switch (ar->event) {
    case LUA_HOOKCALL: {  /* push call onto stack */
      int err = lmprof_stack_push(st->mem_stack, st->alloc_count);
      if (err) {
        lmprof_lstrace_write(L, LMPROF_STACK_DUMP_FILENAME);
        destroy(L, st, TRUE);
        luaL_error(L, "lmprof stack overflow (current size = %d). We consider \
your call chain very big. '%s' contains the full stack trace and '%s' contais \
the memory profile.", LMPROF_STACK_SIZE, LMPROF_STACK_DUMP_FILENAME,
                                         st->output_filename);
      }
      break;
    }
    case LUA_HOOKRET: {  /* in case of alloc, update function */
      if (lmprof_stack_equal(st->mem_stack, st->alloc_count)) {
        int err = lmprof_stack_pop(st->mem_stack);
        if (err) {
          destroy(L, st, TRUE);
          luaL_error(L, "unable to pop from empty stack. you did not call stop \
or called stop in a level 'upper' than start was called.");
        }
      } else {
        size_t total_mem;
        size_t self_mem = lmprof_stack_smart_pop(st->mem_stack, st->alloc_count, &total_mem);
        if(st->tail_parent) {/* TODO: fix bug. last tailcall has wrong parent*/
          st->tail_parent = 0;  /* returning from tailcall, erase flag */
        }
        update(L, st, ar, self_mem, total_mem, LMPROF_UPDATE_CALL);
      }
      break;
    }
    /* push call onto stack and, in case of alloc, update function */
    case LUA_HOOKTAILCALL:
      if (lmprof_stack_equal(st->mem_stack, st->alloc_count)) {
        int err = lmprof_stack_pop(st->mem_stack);
        if (err) {
          destroy(L, st, TRUE);
          luaL_error(L, "unable to pop from empty stack. you did not call stop \
or called stop in a level 'upper' than start was called.");
        }
      } else {
        size_t total_mem;
        size_t self_mem = lmprof_stack_smart_pop(st->mem_stack, st->alloc_count, &total_mem);
        update(L, st, ar, self_mem, total_mem, LMPROF_UPDATE_TAIL_CALL);
      }
      lmprof_stack_push(st->mem_stack, st->alloc_count);
      break;
  }
  st->increment_alloc_count = 1;  /* enable alloc count */
}

static void destroy (lua_State *L, lmprof_State *st, int rm_finalizer) {
  /* restore alloc function if object being destroyed is the current obj */
  void *current;
  lua_getallocf (L, &current);
  if (current == st) {
    lua_setallocf(L, st->la.f, st->la.ud);
  }

  /*
   * 'if' is used to garantee that 'finalizer' does not overwrite a new obj,
   * eventhough it could never happen through the lib API.
   */
  if (rm_finalizer) {
    lua_getfield(L, LUA_REGISTRYINDEX, LMPROF_UD_ALLOC);
    lua_pushnil(L);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
  }

  lmprof_stack_destroy(st->mem_stack);
  lmprof_hash_print(st->func_calls, st->output_filename);
  lmprof_hash_destroy(st->func_calls);
  free(st->output_filename);
  lua_sethook(L, hook, 0, 0);
}

/*
** Called when main program ends.
** Restores lua_State original allocation function.
*/
static int finalize (lua_State *L) {
  lmprof_State *st = lua_touserdata(L, -1);
  destroy(L, st, FALSE);
  return 0;
}

/* Register finalize function as metatable */
static lmprof_State* new_lmprof_state (lua_State *L) {
  lmprof_State *st = (lmprof_State*) lua_newuserdata(L, 
                                               (size_t) sizeof(lmprof_State));
  static lua_Alloc f;
  static void *ud;
  f = lua_getallocf(L, &ud);

  /* create 'alloc' userdata (one ud for each Lua_State) */
  st->la.f = f;
  st->la.ud = ud;

  /* create metatable with finalize function (__gc field) */
  luaL_newmetatable(L, "lmprof_mt");
  lua_pushcfunction(L, finalize);
  lua_setfield(L, -2, "__gc");

  /* set userdata metatable */
  lua_setmetatable(L, -2);

  /* insert userdata into registry table so it cannot be collected */
  lua_setfield(L, LUA_REGISTRYINDEX, LMPROF_UD_ALLOC);

  return st;
}


static void *alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  /* opaque pointer is the library state */
  lmprof_State *st = (lmprof_State *) ud;

  if (nsize == 0) {  /* check lua manual for more details */
    free(ptr);
    return NULL;
  } else if (ptr == NULL) {  /* case (malloc): osize holds object type */
    osize = 0;
  }
  if (nsize > osize && st->increment_alloc_count == 1) {
    st->alloc_count = st->alloc_count + (nsize - osize);
  }
  return realloc(ptr, nsize);
}

static int start (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);

  /* if start has been called before, erase lib */
  if (st != NULL) {
    destroy(L, st, TRUE);
    lua_pushstring(L, "calling lmprof start function twice");
    lua_error(L);
  }

  /* create data_structure and set new alloc function */
  st = new_lmprof_state(L);
  lua_setallocf(L, alloc, (void *) st);

  /* create auxiliary structures and set call/return hook */
  st->alloc_count = 0;
  st->ignore_return = 1;
  st->increment_alloc_count = 0;
  st->tail_parent = 0;
  st->mem_stack = lmprof_stack_create();
  st->func_calls = lmprof_hash_create();

  /* check if there is an output_filename */
  if (lua_gettop(L)) {
    /* strcpy because Lua string could be collected */
    const char *s = luaL_checkstring(L, 1);
    st->output_filename = malloc(strlen(s) + 1);
    strcpy(st->output_filename, s);
  } else {
    /* strcpy to simplify stop calling free, if user passes in both calls */
    const char *s = LMPROF_DEFAULT_OUTPUT_FILENAME;
    st->output_filename = malloc(strlen(s) + 1);
    strcpy(st->output_filename, s);
  }

  /* push parent to enable not using lmprof.stop in main script */
  lmprof_stack_push(st->mem_stack, st->alloc_count);
  /* push current function */
  lmprof_stack_push(st->mem_stack, st->alloc_count);

  lua_sethook(L, hook, LUA_MASKCALL | LUA_MASKRET, 0);
  st->increment_alloc_count = 1;
  return 0;
}

static int stop (lua_State *L) {
  lmprof_State *st = LMPROF_GET_STATE_INFO(L);
  size_t self_mem;
  size_t total_mem;
  lua_Debug ar;

  st->increment_alloc_count = 0;

  /* check if there is an output_filename and update previous value */
  if (lua_gettop(L)) {
    const char *s = luaL_checkstring(L, 1);
    st->output_filename = realloc(st->output_filename, strlen(s) + 1);
    strcpy(st->output_filename, s);
  }

  if (lmprof_stack_size(st->mem_stack) != LMPROF_STOP_QUEUE_SIZE) {
    luaL_error(L, "stop was not called at the same level as start.");
  }
  
  lmprof_stack_pop(st->mem_stack);  /* pop stop own call */

  /* update last function as root */
  self_mem = lmprof_stack_smart_pop(st->mem_stack, st->alloc_count, &total_mem);
  lua_getstack(L, 1, &ar);
  update(L, st, &ar, self_mem, total_mem, LMPROF_UPDATE_ROOT);

  /* 
   * ignore last call on stack. if stop is never called, this last call
   * is updated and finalize clear lmprof.
   */
printf("FUNCTION HOOK CALLS: %ld\n", func_counter);

  destroy(L, st, TRUE);
  return 0;
}

/* luamemprofiler function registration array */
static const luaL_Reg lmprof[] = {
  { "start",  start},
  { "stop",   stop},
  { NULL, NULL }
};

/* register luamemprofiler functions */
LUALIB_API int luaopen_lmprof (lua_State *L) {
  luaL_newlib(L, lmprof);
  return 1;
}

/* }================================================================== */

