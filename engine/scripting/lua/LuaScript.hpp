/*
   Copyright 2022 MacKenzie Strand

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

#pragma once

#include "utilities/Common.h"
#include "core/ModuleBase.hpp"

#include "lua.hpp"

/* Function declaration */
#define CT_LUA_FUNCTION_DECLARE(_NAME) static int _NAME(lua_State* L)

/* Raising an error inside a function */
#define CT_LUA_RAISE_ERROR(_TEXT)                                                        \
   lua_pushstring(L, #_TEXT);                                                            \
   lua_error(L);                                                                         \
   return 0;

/* Fetch arguments */
#define CT_LUA_ARG_IS_NULL(_NUM) lua_isnil(L, _NUM)

#define CT_LUA_ARG_NUMBER(_NUM, _NAME, _TYPE)                                            \
   _TYPE _NAME;                                                                          \
   if (lua_isnumber(L, _NUM)) {                                                          \
      _NAME = (_TYPE)lua_tonumber(L, _NUM);                                              \
   } else {                                                                              \
      CT_LUA_RAISE_ERROR("argument " #_NUM " is not a number")                           \
   }

#define CT_LUA_ARG_BOOL(_NUM, _NAME)                                                     \
   bool _NAME;                                                                           \
   if (lua_isboolean(L, _NUM)) {                                                         \
      _NAME = (bool)lua_toboolean(L, _NUM);                                              \
   } else {                                                                              \
      CT_LUA_RAISE_ERROR("argument " #_NUM " is not a bool")                             \
   }

#define CT_LUA_ARG_TEMP_STRING(_NUM, _NAME)                                              \
   const char* _NAME;                                                                    \
   if (lua_isstring(L, _NUM)) {                                                          \
      _NAME = lua_tostring(L, _NUM);                                                     \
   } else {                                                                              \
      CT_LUA_RAISE_ERROR("argument " #_NUM " is not a string")                           \
   }

#define CT_LUA_ARG_OBJECT_PTR(_NUM, _NAME, _TYPE)                                        \
   _TYPE* _NAME;                                                                         \
   if (lua_isuserdata(L, _NUM)) {                                                        \
      _NAME = (_TYPE*)luaL_checkudata(L, _NUM, #_TYPE);                                  \
      if (!_NAME) {                                                                      \
         CT_LUA_RAISE_ERROR("argument " #_NUM                                            \
                            " is null or not a reference to a " #_TYPE);                 \
      }                                                                                  \
   } else {                                                                              \
      CT_LUA_RAISE_ERROR("argument " #_NUM " is not a reference to a " #_TYPE);          \
   }

#define CT_LUA_ARG_OBJECT(_NUM, _NAME, _TYPE)                                            \
   _TYPE _NAME;                                                                          \
   {                                                                                     \
      CT_LUA_ARG_OBJECT_PTR(_NUM, _tmpPtr, _TYPE);                                       \
      _NAME = *_tmpPtr;                                                                  \
   }

/* Set return value */
#define CT_LUA_RETURN_VOID() return 0;
#define CT_LUA_RETURN_NUMBER(_VALUE)                                                     \
   lua_pushnumber(L, (lua_Number)_VALUE);                                                \
   return 1;
#define CT_LUA_RETURN_BOOL(_VALUE)                                                       \
   lua_pushboolean(L, (int)_VALUE);                                                      \
   return 1;
#define CT_LUA_RETURN_STRING_COPY(_VALUE)                                                \
   lua_pushstring(L, _VALUE);                                                            \
   return 1;

#define CT_LUA_RETURN_OBJECT_PTR(_TYPE_STR, _VALUE)                                      \
   lua_pushlightuserdata(L, (void*)_VALUE);                                              \
   luaL_setmetatable(L, _TYPE_STR);                                                      \
   return 1;
#define CT_LUA_RETURN_OBJECT_POD_COPY(_TYPE_STR, _VALUE)                                 \
   {                                                                                     \
      void* _dest = lua_newuserdata(L, sizeof(_VALUE));                                  \
      memcpy(_dest, &_VALUE, sizeof(_VALUE));                                            \
   }                                                                                     \
   luaL_setmetatable(L, _TYPE_STR);                                                      \
   return 1;

class CT_API ctLuaContext {
public:
   ctResults Startup(bool trusted);
   ctResults Shutdown();

   ctResults LoadFromBuffer(const char* data, size_t size, const char* name);
   ctResults LoadFromFile(const char* path);
   ctResults Compile(ctDynamicArray<uint8_t>& output);

   ctResults RunScript();

   ctResults RegisterType(const char* name, lua_CFunction garbageCollect = NULL);
   ctResults RegisterFunction(const char* name, lua_CFunction function);
   ctResults CallFunction(const char* name, const char* signature, ...);

   lua_State* L;
};