/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "LuaScript.hpp"

/* Wrapped libraries */
extern "C" {
extern int luaopen_scene(lua_State* L);
}

/* Debug logging */
static int ctLuaPrint(lua_State* L) {
   ZoneScoped;
   int argc = lua_gettop(L);
   ctStringUtf8 str = "";
   for (int i = 1; i <= argc; i++) {
      str += lua_tostring(L, i);
   }
   ctDebugLog("%s", str.CStr());
   return 0;
}
static const struct luaL_Reg ctLuaPrintLib[] = {{"print", ctLuaPrint}, {NULL, NULL}};

ctResults ctLuaContext::Startup(bool trusted) {
   ZoneScoped;
   /* Open Lua */
   L = lua_open();
   if (!L) { return CT_FAILURE_UNKNOWN; }

   /* Standard library */
   luaopen_base(L);
   luaopen_table(L);
   luaopen_string(L);
   luaopen_string_buffer(L);
   luaopen_math(L);
   luaopen_bit(L);

   /* Sensitive libraries */
   if (trusted) {
      luaopen_os(L);
      luaopen_io(L);
      luaopen_ffi(L);
      luaopen_jit(L);
      luaopen_package(L);
      luaopen_debug(L);
   }

   /* Override print */
   lua_getglobal(L, "_G");
   luaL_setfuncs(L, ctLuaPrintLib, 0);
   lua_pop(L, 1);
   return CT_SUCCESS;
}

ctResults ctLuaContext::Shutdown() {
   ZoneScoped;
   if (!L) { return CT_FAILURE_NOT_UPDATABLE; }
   lua_close(L);
   return CT_SUCCESS;
}

ctResults ctLuaContext::OpenEngineLibrary(const char* name) {
   ZoneScoped;
   if (!L) { return CT_FAILURE_NOT_UPDATABLE; }

   if (ctCStrEql(name, "scene")) { luaopen_scene(L); }
   return CT_SUCCESS;
}

ctResults ctLuaContext::LoadFromBuffer(const char* data, size_t size, const char* name) {
   ZoneScoped;
   if (!L) { return CT_FAILURE_NOT_UPDATABLE; }
   int result = luaL_loadbuffer(L, data, size, name);
   if (!result) {
      return CT_SUCCESS;
   } else if (result == LUA_ERRSYNTAX) {
      return CT_FAILURE_SYNTAX_ERROR;
   }
   return CT_FAILURE_UNKNOWN;
}

ctResults ctLuaContext::LoadFromFile(const char* path) {
   ZoneScoped;
   if (!L) { return CT_FAILURE_NOT_UPDATABLE; }
   int result = luaL_loadfile(L, path);
   if (!result) {
      return CT_SUCCESS;
   } else if (result == LUA_ERRSYNTAX) {
      ctDebugError("Syntax error in \"%s\": %s", path, lua_tostring(L, -1));
      return CT_FAILURE_SYNTAX_ERROR;
   } else if (result == LUA_ERRFILE) {
      ctDebugError("Failed to load file: %s", path);
      return CT_FAILURE_INACCESSIBLE;
   }
   return CT_FAILURE_UNKNOWN;
}

ctResults ctLuaContext::RunScript() {
   ZoneScoped;
   if (lua_pcall(L, 0, 0, 0) != 0) {
      ctDebugError("ERROR RUNNING LUA SCRIPT: %s", lua_tostring(L, -1));
      return CT_FAILURE_RUNTIME_ERROR;
   }
   return CT_SUCCESS;
}

/* https://www.lua.org/pil/25.3.html */
ctResults ctLuaContext::CallFunction(const char* function, const char* signature, ...) {
   ZoneScoped;
   va_list vlist;
   int narg, nres;
   va_start(vlist, signature);

   /* Get the function */
   lua_getglobal(L, function);

   /* Push arguements */
   narg = 0;
   while (*signature) {
      switch (*signature++) {
         case 'd': lua_pushnumber(L, va_arg(vlist, double)); break;
         case 'i': lua_pushnumber(L, va_arg(vlist, int)); break;
         case 's': lua_pushstring(L, va_arg(vlist, const char*)); break;
         case 'u': lua_pushlightuserdata(L, va_arg(vlist, void*)); break;
         case ':': goto endwhile;
         default: return CT_FAILURE_INVALID_PARAMETER; break;
      }
      narg++;
      luaL_checkstack(L, 1, "Too many arguments");
   }
endwhile:

   /* Call the function */
   nres = (int)strlen(signature);
   if (lua_pcall(L, narg, nres, 0) != 0) {
      ctDebugError(
        "ERROR CALLING LUA FUNCTION \"%s\": %s", function, lua_tostring(L, -1));
      return CT_FAILURE_RUNTIME_ERROR;
   }

   /* Get results (if there are any) */
   nres = -nres;
   while (*signature) {
      switch (*signature++) {
         case 'd':
            if (!lua_isnumber(L, nres)) { return CT_FAILURE_TYPE_ERROR; }
            *va_arg(vlist, double*) = lua_tonumber(L, nres);
         case 'i':
            if (!lua_isnumber(L, nres)) { return CT_FAILURE_TYPE_ERROR; }
            *va_arg(vlist, double*) = lua_tonumber(L, nres);
         case 's':
            if (!lua_isstring(L, nres)) { return CT_FAILURE_TYPE_ERROR; }
            *va_arg(vlist, ctStringUtf8*) = lua_tostring(L, nres);
         default: return CT_FAILURE_INVALID_PARAMETER; break;
      }
      nres++;
   }
   va_end(vlist);
   return CT_SUCCESS;
}
