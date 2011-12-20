/*
 * The glue between lualatex and peg-multimarkdown.
 *
 * Author: Marko Mahnič
 * Created: December 2011
 */

#include <stdio.h>
/* extern "C" { */
#include "lua.h"
#include "lauxlib.h"
/* } */

#include "markdown_lib.h"

#define PROJECT_TABLENAME "multimarkdownlualib"
#ifdef WIN32
#define LUA_API __declspec(dllexport)
#else
#define LUA_API
#endif

/* extern "C" { */
   int LUA_API luaopen_multimarkdownlualib (lua_State *L);
/* } */

static int lualatexmarkdown (lua_State *L) {
   const char* text = luaL_checkstring(L, 1);
   int extensions = 0;
   char* out = markdown_to_string(text, extensions, LATEX_FORMAT);
   /*printf("\n-------\n%s\n-------\n", out);*/
   lua_pushstring(L, out);
   free(out);
   return 1;
}

int LUA_API luaopen_multimarkdownlualib (lua_State *L) {
   struct luaL_reg driver[] = {
      /* string tolatex(string mkdText) */
      {"tolatex", lualatexmarkdown},
      /* TODO: string totex(string mkdText) */
      /* TODO: string tocontext(string mkdText) */
      {NULL, NULL},
   };
   luaL_openlib (L, "multimarkdownlualib", driver, 0);
   return 1;
}
