/*
** Lua binding: extensions
** Generated automatically by tolua++-1.0.93 on Wed May 15 13:59:27 2013.
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
TOLUA_API int  tolua_extensions_open (lua_State* tolua_S);

#include "LuaCocos2dExtensions.h"
#include "physics_nodes/CCPhysicsSprite.h"
#include "physics_nodes/CCPhysicsNode.h"
#include "tolua_fix.h"
USING_NS_CC;
USING_NS_CC_EXT;

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CCSpriteFrame");
 tolua_usertype(tolua_S,"CCRect");
 tolua_usertype(tolua_S,"CCPhysicsNode");
 tolua_usertype(tolua_S,"CCSprite");
 tolua_usertype(tolua_S,"CCPhysicsSprite");
 tolua_usertype(tolua_S,"b2Body");
 tolua_usertype(tolua_S,"CCTexture2D");
 tolua_usertype(tolua_S,"CCNode");
}

/* method: getB2Body of class  CCPhysicsNode */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsNode_getB2Body00
static int tolua_extensions_CCPhysicsNode_getB2Body00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CCPhysicsNode",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CCPhysicsNode* self = (const CCPhysicsNode*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getB2Body'", NULL);
#endif
  {
   b2Body* tolua_ret = (b2Body*)  self->getB2Body();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"b2Body");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getB2Body'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setB2Body of class  CCPhysicsNode */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsNode_setB2Body00
static int tolua_extensions_CCPhysicsNode_setB2Body00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CCPhysicsNode",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"b2Body",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCPhysicsNode* self = (CCPhysicsNode*)  tolua_tousertype(tolua_S,1,0);
  b2Body* pBody = ((b2Body*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setB2Body'", NULL);
#endif
  {
   self->setB2Body(pBody);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setB2Body'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPTMRatio of class  CCPhysicsNode */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsNode_getPTMRatio00
static int tolua_extensions_CCPhysicsNode_getPTMRatio00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CCPhysicsNode",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CCPhysicsNode* self = (const CCPhysicsNode*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPTMRatio'", NULL);
#endif
  {
   float tolua_ret = (float)  self->getPTMRatio();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPTMRatio'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPTMRatio of class  CCPhysicsNode */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsNode_setPTMRatio00
static int tolua_extensions_CCPhysicsNode_setPTMRatio00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CCPhysicsNode",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCPhysicsNode* self = (CCPhysicsNode*)  tolua_tousertype(tolua_S,1,0);
  float fPTMRatio = ((float)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPTMRatio'", NULL);
#endif
  {
   self->setPTMRatio(fPTMRatio);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPTMRatio'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: create of class  CCPhysicsNode */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsNode_create00
static int tolua_extensions_CCPhysicsNode_create00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsNode",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CCPhysicsNode* tolua_ret = (CCPhysicsNode*)  CCPhysicsNode::create();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CCPhysicsNode");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'create'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getB2Body of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_getB2Body00
static int tolua_extensions_CCPhysicsSprite_getB2Body00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CCPhysicsSprite* self = (const CCPhysicsSprite*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getB2Body'", NULL);
#endif
  {
   b2Body* tolua_ret = (b2Body*)  self->getB2Body();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"b2Body");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getB2Body'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setB2Body of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_setB2Body00
static int tolua_extensions_CCPhysicsSprite_setB2Body00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"b2Body",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCPhysicsSprite* self = (CCPhysicsSprite*)  tolua_tousertype(tolua_S,1,0);
  b2Body* pBody = ((b2Body*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setB2Body'", NULL);
#endif
  {
   self->setB2Body(pBody);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setB2Body'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPTMRatio of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_getPTMRatio00
static int tolua_extensions_CCPhysicsSprite_getPTMRatio00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CCPhysicsSprite* self = (const CCPhysicsSprite*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPTMRatio'", NULL);
#endif
  {
   float tolua_ret = (float)  self->getPTMRatio();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPTMRatio'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPTMRatio of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_setPTMRatio00
static int tolua_extensions_CCPhysicsSprite_setPTMRatio00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCPhysicsSprite* self = (CCPhysicsSprite*)  tolua_tousertype(tolua_S,1,0);
  float fPTMRatio = ((float)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPTMRatio'", NULL);
#endif
  {
   self->setPTMRatio(fPTMRatio);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPTMRatio'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: createWithTexture of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_createWithTexture00
static int tolua_extensions_CCPhysicsSprite_createWithTexture00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CCTexture2D",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCTexture2D* pTexture = ((CCTexture2D*)  tolua_tousertype(tolua_S,2,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::createWithTexture(pTexture);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'createWithTexture'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: createWithTexture of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_createWithTexture01
static int tolua_extensions_CCPhysicsSprite_createWithTexture01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CCTexture2D",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"CCRect",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  CCTexture2D* pTexture = ((CCTexture2D*)  tolua_tousertype(tolua_S,2,0));
  CCRect rect = *((CCRect*)  tolua_tousertype(tolua_S,3,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::createWithTexture(pTexture,rect);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
tolua_lerror:
 return tolua_extensions_CCPhysicsSprite_createWithTexture00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: createWithSpriteFrame of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_createWithSpriteFrame00
static int tolua_extensions_CCPhysicsSprite_createWithSpriteFrame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CCSpriteFrame",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CCSpriteFrame* pSpriteFrame = ((CCSpriteFrame*)  tolua_tousertype(tolua_S,2,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::createWithSpriteFrame(pSpriteFrame);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'createWithSpriteFrame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: createWithSpriteFrameName of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_createWithSpriteFrameName00
static int tolua_extensions_CCPhysicsSprite_createWithSpriteFrameName00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* pszSpriteFrameName = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::createWithSpriteFrameName(pszSpriteFrameName);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'createWithSpriteFrameName'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: create of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_create00
static int tolua_extensions_CCPhysicsSprite_create00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"CCRect",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* pszFileName = ((const char*)  tolua_tostring(tolua_S,2,0));
  CCRect rect = *((CCRect*)  tolua_tousertype(tolua_S,3,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::create(pszFileName,rect);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'create'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: create of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_create01
static int tolua_extensions_CCPhysicsSprite_create01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const char* pszFileName = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::create(pszFileName);
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
tolua_lerror:
 return tolua_extensions_CCPhysicsSprite_create00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: create of class  CCPhysicsSprite */
#ifndef TOLUA_DISABLE_tolua_extensions_CCPhysicsSprite_create02
static int tolua_extensions_CCPhysicsSprite_create02(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CCPhysicsSprite",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  {
   CCPhysicsSprite* tolua_ret = (CCPhysicsSprite*)  CCPhysicsSprite::create();
    int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
    int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
    toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCPhysicsSprite");
  }
 }
 return 1;
tolua_lerror:
 return tolua_extensions_CCPhysicsSprite_create01(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_extensions_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_cclass(tolua_S,"CCPhysicsNode","CCPhysicsNode","CCNode",NULL);
  tolua_beginmodule(tolua_S,"CCPhysicsNode");
   tolua_function(tolua_S,"getB2Body",tolua_extensions_CCPhysicsNode_getB2Body00);
   tolua_function(tolua_S,"setB2Body",tolua_extensions_CCPhysicsNode_setB2Body00);
   tolua_function(tolua_S,"getPTMRatio",tolua_extensions_CCPhysicsNode_getPTMRatio00);
   tolua_function(tolua_S,"setPTMRatio",tolua_extensions_CCPhysicsNode_setPTMRatio00);
   tolua_function(tolua_S,"create",tolua_extensions_CCPhysicsNode_create00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CCPhysicsSprite","CCPhysicsSprite","CCSprite",NULL);
  tolua_beginmodule(tolua_S,"CCPhysicsSprite");
   tolua_function(tolua_S,"getB2Body",tolua_extensions_CCPhysicsSprite_getB2Body00);
   tolua_function(tolua_S,"setB2Body",tolua_extensions_CCPhysicsSprite_setB2Body00);
   tolua_function(tolua_S,"getPTMRatio",tolua_extensions_CCPhysicsSprite_getPTMRatio00);
   tolua_function(tolua_S,"setPTMRatio",tolua_extensions_CCPhysicsSprite_setPTMRatio00);
   tolua_function(tolua_S,"createWithTexture",tolua_extensions_CCPhysicsSprite_createWithTexture00);
   tolua_function(tolua_S,"createWithTexture",tolua_extensions_CCPhysicsSprite_createWithTexture01);
   tolua_function(tolua_S,"createWithSpriteFrame",tolua_extensions_CCPhysicsSprite_createWithSpriteFrame00);
   tolua_function(tolua_S,"createWithSpriteFrameName",tolua_extensions_CCPhysicsSprite_createWithSpriteFrameName00);
   tolua_function(tolua_S,"create",tolua_extensions_CCPhysicsSprite_create00);
   tolua_function(tolua_S,"create",tolua_extensions_CCPhysicsSprite_create01);
   tolua_function(tolua_S,"create",tolua_extensions_CCPhysicsSprite_create02);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_extensions (lua_State* tolua_S) {
 return tolua_extensions_open(tolua_S);
};
#endif

