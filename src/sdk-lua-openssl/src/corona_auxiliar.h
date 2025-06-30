// albert: I modified this to avoid a collision with luasocket/src/auxiliar.h.
#ifndef _CORONA_AUXILIAR_H
#define _CORONA_AUXILIAR_H

#include "lua.h"
#include "lauxlib.h"
#if LUA_VERSION_NUM>501
#define luaL_reg luaL_Reg
#endif

int auxiliar_isclass(lua_State *L, const char *classname, int objidx);
int auxiliar_isgroup(lua_State *L, const char *groupname, int objidx);

#endif /* _CORONA_AUXILIAR_H */
