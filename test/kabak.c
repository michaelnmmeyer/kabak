#include <lua.h>
#include <lauxlib.h>
#include "../kabak.h"

static int kb_lua_transform(lua_State *lua)
{
   int argc = lua_gettop(lua);
   unsigned opts = luaL_optnumber(lua, 1, KB_NFC);

   struct kabak buf = KB_INIT;
   for (int i = 2; i <= argc; i++) {
      if (!lua_isstring(lua, i)) {
         kb_fini(&buf);
         return luaL_error(lua, "value at %d is not a string", i);
      }
      size_t len;
      const char *str = lua_tolstring(lua, i, &len);
      kb_transform(&buf, str, len, opts);
   }
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
   
   $(NFC)
   $(NFKC)
   #undef $

   return 1;
}
