#include <string.h>
#include <stdio.h>

#include "corona_auxiliar.h"
#include "auxiliar.h"
#if LUA_VERSION_NUM==501
#define lua_rawlen lua_strlen
#define luaL_typeerror luaL_typerror
#endif

int auxiliar_isclass(lua_State *L, const char *classname, int objidx) {
    void *p = lua_touserdata(L, objidx);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, objidx)) {  /* does it have a metatable? */
            lua_getfield(L, LUA_REGISTRYINDEX, classname);  /* get correct metatable */
            if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
                lua_pop(L, 2);  /* remove both metatables */
                return 1;
            }
        }
    }
    return 0;
}

int auxiliar_isgroup(lua_State *L, const char *groupname, int objidx) {
    void *data = auxiliar_getgroupudata(L, groupname, objidx);
    return data!=NULL;
}
