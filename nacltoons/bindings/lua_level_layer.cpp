/*
** Lua binding: level_layer
** Generated automatically by tolua++-1.0.93 on Wed Mar 27 14:15:50 2013.
*/

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_level_layer_open (lua_State* tolua_S);

#include "lua_level_layer.h"
#include "level_layer.h"
#include "game_manager.h"
#include "tolua_fix.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"b2Vec2");
 tolua_usertype(tolua_S,"CCLayerColor");
 tolua_usertype(tolua_S,"LUA_FUNCTION");
 tolua_usertype(tolua_S,"GameManager");
 tolua_usertype(tolua_S,"b2World");
 tolua_usertype(tolua_S,"LevelLayer");
}

/* method: GetWorld of class  LevelLayer */
#ifndef TOLUA_DISABLE_tolua_level_layer_LevelLayer_GetWorld00
static int tolua_level_layer_LevelLayer_GetWorld00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"LevelLayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  LevelLayer* self = (LevelLayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetWorld'", NULL);
#endif
  {
   b2World* tolua_ret = (b2World*)  self->GetWorld();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"b2World");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetWorld'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: LevelComplete of class  LevelLayer */
#ifndef TOLUA_DISABLE_tolua_level_layer_LevelLayer_LevelComplete00
static int tolua_level_layer_LevelLayer_LevelComplete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"LevelLayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  LevelLayer* self = (LevelLayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'LevelComplete'", NULL);
#endif
  {
   self->LevelComplete();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LevelComplete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ToggleDebug of class  LevelLayer */
#ifndef TOLUA_DISABLE_tolua_level_layer_LevelLayer_ToggleDebug00
static int tolua_level_layer_LevelLayer_ToggleDebug00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"LevelLayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  LevelLayer* self = (LevelLayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ToggleDebug'", NULL);
#endif
  {
   self->ToggleDebug();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ToggleDebug'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: FindBodiesAt of class  LevelLayer */
#ifndef TOLUA_DISABLE_tolua_level_layer_LevelLayer_FindBodiesAt00
static int tolua_level_layer_LevelLayer_FindBodiesAt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"LevelLayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"b2Vec2",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !toluafix_isfunction(tolua_S,3,"LUA_FUNCTION",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  LevelLayer* self = (LevelLayer*)  tolua_tousertype(tolua_S,1,0);
  b2Vec2* pos = ((b2Vec2*)  tolua_tousertype(tolua_S,2,0));
  LUA_FUNCTION callback = ( toluafix_ref_function(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'FindBodiesAt'", NULL);
#endif
  {
   self->FindBodiesAt(pos,callback);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FindBodiesAt'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: sharedManager of class  GameManager */
#ifndef TOLUA_DISABLE_tolua_level_layer_GameManager_sharedManager00
static int tolua_level_layer_GameManager_sharedManager00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GameManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   GameManager* tolua_ret = (GameManager*)  GameManager::sharedManager();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GameManager");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sharedManager'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Restart of class  GameManager */
#ifndef TOLUA_DISABLE_tolua_level_layer_GameManager_Restart00
static int tolua_level_layer_GameManager_Restart00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameManager* self = (GameManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Restart'", NULL);
#endif
  {
   self->Restart();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Restart'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: LoadLevel of class  GameManager */
#ifndef TOLUA_DISABLE_tolua_level_layer_GameManager_LoadLevel00
static int tolua_level_layer_GameManager_LoadLevel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameManager* self = (GameManager*)  tolua_tousertype(tolua_S,1,0);
  int level_number = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'LoadLevel'", NULL);
#endif
  {
   self->LoadLevel(level_number);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LoadLevel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: LoadGame of class  GameManager */
#ifndef TOLUA_DISABLE_tolua_level_layer_GameManager_LoadGame00
static int tolua_level_layer_GameManager_LoadGame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GameManager",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GameManager* self = (GameManager*)  tolua_tousertype(tolua_S,1,0);
  const char* folder = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'LoadGame'", NULL);
#endif
  {
   self->LoadGame(folder);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LoadGame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_level_layer_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_cclass(tolua_S,"LevelLayer","LevelLayer","CCLayerColor",NULL);
  tolua_beginmodule(tolua_S,"LevelLayer");
   tolua_function(tolua_S,"GetWorld",tolua_level_layer_LevelLayer_GetWorld00);
   tolua_function(tolua_S,"LevelComplete",tolua_level_layer_LevelLayer_LevelComplete00);
   tolua_function(tolua_S,"ToggleDebug",tolua_level_layer_LevelLayer_ToggleDebug00);
   tolua_function(tolua_S,"FindBodiesAt",tolua_level_layer_LevelLayer_FindBodiesAt00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"GameManager","GameManager","",NULL);
  tolua_beginmodule(tolua_S,"GameManager");
   tolua_function(tolua_S,"sharedManager",tolua_level_layer_GameManager_sharedManager00);
   tolua_function(tolua_S,"Restart",tolua_level_layer_GameManager_Restart00);
   tolua_function(tolua_S,"LoadLevel",tolua_level_layer_GameManager_LoadLevel00);
   tolua_function(tolua_S,"LoadGame",tolua_level_layer_GameManager_LoadGame00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_level_layer (lua_State* tolua_S) {
 return tolua_level_layer_open(tolua_S);
};
#endif

