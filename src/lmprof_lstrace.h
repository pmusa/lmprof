#ifndef lmprof_lstrace_h
#define lmprof_lstrace_h

const char*  lmprof_lstrace_getfuncinfo  (lua_State *L, lua_Debug *ar);
void         lmprof_lstrace_write        (lua_State *L, const char *filename);

#endif
