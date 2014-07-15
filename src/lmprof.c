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

static lmprof_stack *mem_stack;
static lmprof_hash  *func_calls;
static int ignore_first_return;


static lmprof_hash insert(lua_State *L, uintptr_t function, uintptr_t parent,
                                                     lua_Debug *ar) {
  const char *name;
  lua_getinfo(L, "n", ar);
  if (ar-> name != NULL) {
//printf("name: %s - namewhat: %s\n", ar->name, ar->namewhat);
    name = ar->name;
  } else {
    lua_getinfo(L, "S", ar);
//printf("source: %s - short_src: %s - linedefined: %d - lastlinedefined: %d - what: %s\n", ar->source, ar->short_src, ar->linedefined, ar->lastlinedefined, ar->what);    
    name = ar->what;
  }
  lua_settop(L, -1);
  return lmprof_hash_insert(func_calls, function, parent, name);
}

static void XXX(lua_State *L, lua_Debug * far, int omem, int nmem) {
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
    int omem = lua_gc (L, LUA_GCCOUNT, 0);
    int nok = lmprof_stack_push(mem_stack, omem);
    if (nok) {
      lua_pushstring(L, "call stack is too big");
      lua_error(L);
    }
  } else if (ar->event == LUA_HOOKRET) {
    int nmem = lua_gc (L, LUA_GCCOUNT, 0);
    int omem = lmprof_stack_pop(mem_stack);
    if (nmem > omem) {
      XXX(L, ar, omem, nmem);
      //printf("Mudanca de Tamanho: %d - %d - %d\n", omem, nmem, nmem - omem);
    } else if (omem < nmem) {  /* gc executed */
      printf("gc executed. ");
      printf("Mudanca de Tamanho: %d - %d - %d\n", omem, nmem, nmem - omem);
    }
  }
}

static int lmprof_start(lua_State *L) {
  ignore_first_return = 1;
  mem_stack = lmprof_stack_create();
  func_calls = lmprof_hash_create();
  lua_sethook (L, lmprof_hook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 0;
}

static int lmprof_stop(lua_State *L) {
  lmprof_stack_destroy(mem_stack);
  lmprof_hash_print(func_calls, "lmprof_output.txt");
  lmprof_hash_destroy(func_calls);
  lua_sethook (L, lmprof_hook, 0, 0);
  return 0;
}

/* luamemprofiler function registration array */
static const luaL_Reg lmprof[] = {
  { "start", lmprof_start},
  { "stop", lmprof_stop},
  { NULL, NULL }
};

/* register luamemprofiler functions */
LUALIB_API int luaopen_lmprof (lua_State *L) {
  luaL_newlib(L, lmprof);
  return 1;
}

