#include <lua.h>
#include <lauxlib.h>
#include "../kabak.h"

static int kb_lua_transform(lua_State *lua)
{
   size_t len;
   const char *str = luaL_checklstring(lua, 1, &len);
   unsigned opts = luaL_optnumber(lua, 2, KB_XNFC);
   
   struct kabak buf = KB_INIT;
   kb_transform(&buf, str, len, opts);
   lua_pushlstring(lua, buf.str, buf.len);
   kb_fini(&buf);
   return 1;
}

int luaopen_kabak(lua_State *lua)
{
   const luaL_Reg lib[] = {
      {"transform", kb_lua_transform},
      {NULL, 0},
   };
   luaL_newlib(lua, lib);
   lua_pushstring(lua, KB_VERSION);
   lua_setfield(lua, -2, "VERSION");

   #define $(name) lua_pushnumber(lua, KB_##name); lua_setfield(lua, -2, #name);
   $(COMPOSE)
   $(DECOMPOSE)
   $(COMPAT)
   $(LUMP)
   $(CASE_FOLD)
   
   $(STRIP_IGNORABLE)
   $(STRIP_UNKNOWN)
   $(STRIP_DIACRITIC)
   
   $(XNFC)
   $(XNFKC)
   $(XCASE_FOLD)
   $(XSTRIP_DIACRITIC)
   #undef $

   return 1;
}
