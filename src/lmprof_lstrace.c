#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdint.h>

static int findfield (lua_State *L, int objidx, int level) {
  if (level == 0 || !lua_istable(L, -1))
    return 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  while (lua_next(L, -2)) {  /* for each pair in table */
    if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      if (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        return 1;
      }
      else if (findfield(L, objidx, level - 1)) {  /* try recursively */
        lua_remove(L, -2);  /* remove table (but keep name) */
        lua_pushliteral(L, ".");
        lua_insert(L, -2);  /* place '.' between the two names */
        lua_concat(L, 3);
        return 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  return 0;  /* not found */
}

static int pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  int top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_pushglobaltable(L);
  if (findfield(L, top + 1, 2)) {
    lua_copy(L, -1, top + 1);  /* move name to proper place */
    lua_pop(L, 2);  /* remove pushed values */
    return 1;
  }
  else {
    lua_settop(L, top);  /* remove function and global table */
    return 0;
  }
}

static void pushfuncname (lua_State *L, lua_Debug *ar) {
  if (*ar->namewhat != '\0')  /* is there a name? */
    lua_pushfstring(L, "function " LUA_QS, ar->name);
  else if (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  else if (*ar->what == 'C') {
    if (pushglobalfuncname(L, ar)) {
      lua_pushfstring(L, "function " LUA_QS, lua_tostring(L, -1));
      lua_remove(L, -2);  /* remove name */
    }
    else
      lua_pushliteral(L, "?");
  }
  else
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
}

const char* lmprof_lstrace_getfuncinfo (lua_State *L, lua_Debug *ar) {
  const char *funcinfo;
  int top = lua_gettop(L);
  lua_getinfo(L, "Slnt", ar);
  if (*ar->namewhat != '\0') { /* is there a name? */
    lua_pushstring(L, ar->name);
    if (ar->linedefined > 0) {
      lua_pushfstring(L, " (%s:%d)", ar->short_src, ar->linedefined);
    } else {
      lua_pushfstring(L, " %s", ar->short_src);
    }
  } else if (*ar->what == 'm') { /* main? */
    lua_pushliteral(L, "main chunk");
    lua_pushfstring(L, " (%s)", ar->short_src);
  } else if (*ar->what == 'C') {
    if (pushglobalfuncname(L, ar)) {
      lua_pushstring(L, lua_tostring(L, -1));
      lua_remove(L, -2);  /* remove name */
    } else {
      lua_pushliteral(L, "?");
    }
    lua_pushfstring(L, " %s", ar->short_src);
  } else {
    lua_pushliteral(L, "?");
    lua_pushfstring(L, " (%s:%d)", ar->short_src, ar->linedefined,
                                                  ar->currentline);
  }
  lua_concat(L, lua_gettop(L) - top);
  funcinfo = lua_tostring(L, -1); /* do not pop the string - only after use */
  return funcinfo;
}

const char* lmprof_lstrace_gettracefuncinfo (lua_State *L, lua_Debug *ar) {
  const char *funcinfo;
  int top = lua_gettop(L);
  lua_getinfo(L, "Slnt", ar);
  lua_pushfstring(L, "%s:", ar->short_src);
  if (ar->currentline > 0)
    lua_pushfstring(L, "%d:", ar->currentline);
  lua_pushliteral(L, " in ");
  pushfuncname(L, ar);
  if (ar->istailcall) 
    lua_pushliteral(L, "(...tail calls...)");
  lua_concat(L, lua_gettop(L) - top);
  funcinfo = lua_tostring(L, -1); /* do not pop the string - only after use */
  return funcinfo;
}


int lmprof_lstrace_write (lua_State *L, const char *filename) {
  lua_Debug ar;
  int level = 1;
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    return 0;
  }

  fprintf(f, "%s", "FULL stack traceback:");

  while (lua_getstack(L, level++, &ar)) {
    const char *info;
    uintptr_t fref;
    lua_getinfo(L, "f", &ar);
    fref = (uintptr_t) lua_topointer(L, -1);
    info = lmprof_lstrace_gettracefuncinfo(L, &ar);
    fprintf(f, "\n\t%s - reference: \"%lu\"", info, fref);
    lua_pop(L, 1); /* pop string after use */
  }

  fclose(f);

  return 1;
}

