/*--------------------------------------------------------------------------
 * LuaSec 0.8
 *
 * Copyright (C) 2006-2019 Bruno Silvestre.
 *
 *--------------------------------------------------------------------------*/

#include "config.h"
#include "compat.h"
#include "luasec-options.h"
#include "ec.h"

/**
 * Registre the module.
 */
LSEC_API int luaopen_ssl_config(lua_State *L)
{
  ssl_option_t *opt;

  lua_newtable(L);

  // Options
  lua_pushstring(L, "options");
  lua_newtable(L);
  for (opt = ssl_options; opt->name; opt++) {
    lua_pushstring(L, opt->name);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);
  }
  lua_rawset(L, -3);

  // Protocols
  lua_pushstring(L, "protocols");
  lua_newtable(L);

  lua_pushstring(L, "tlsv1");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
  lua_pushstring(L, "tlsv1_1");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
  lua_pushstring(L, "tlsv1_2");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
#if defined(TLS1_3_VERSION)
  lua_pushstring(L, "tlsv1_3");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
#endif

  lua_rawset(L, -3);

  // Algorithms
  lua_pushstring(L, "algorithms");
  lua_newtable(L);

#ifndef OPENSSL_NO_EC
  lua_pushstring(L, "ec");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
#endif
  lua_rawset(L, -3);
 
  // Curves
  lua_pushstring(L, "curves");
  lsec_get_curves(L);
  lua_rawset(L, -3);

  // Capabilities
  lua_pushstring(L, "capabilities");
  lua_newtable(L);

  // ALPN
  lua_pushstring(L, "alpn");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);

#ifndef OPENSSL_NO_EC
  lua_pushstring(L, "curves_list");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
  lua_pushstring(L, "ecdh_auto");
  lua_pushboolean(L, 1);
  lua_rawset(L, -3);
#endif
  lua_rawset(L, -3);

  return 1;
}
