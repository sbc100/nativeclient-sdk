/*
** Lua binding: physics_layer
** Generated automatically by tolua++-1.0.93 on Mon Feb 25 16:17:52 2013.
*/
#include "lua_physics_layer.h"
#include "physics_layer.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CCLayerColor");
 tolua_usertype(tolua_S,"PhysicsLayer");
 tolua_usertype(tolua_S,"b2World");
}

/* method: GetWorld of class  PhysicsLayer */
#ifndef TOLUA_DISABLE_tolua_physics_layer_PhysicsLayer_GetWorld00
static int tolua_physics_layer_PhysicsLayer_GetWorld00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"PhysicsLayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  PhysicsLayer* self = (PhysicsLayer*)  tolua_tousertype(tolua_S,1,0);
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

/* method: GetWorld of class  PhysicsLayer */
#ifndef TOLUA_DISABLE_tolua_physics_layer_PhysicsLayer_LevelComplete00
static int tolua_physics_layer_PhysicsLayer_LevelComplete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"PhysicsLayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  PhysicsLayer* self = (PhysicsLayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetWorld'", NULL);
#endif
  {
    self->LevelComplete();
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LevelComplete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_physics_layer_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_cclass(tolua_S,"PhysicsLayer","PhysicsLayer","CCLayerColor",NULL);
  tolua_beginmodule(tolua_S,"PhysicsLayer");
   tolua_function(tolua_S,"GetWorld",tolua_physics_layer_PhysicsLayer_GetWorld00);
   tolua_function(tolua_S,"LevelComplete",tolua_physics_layer_PhysicsLayer_LevelComplete00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_physics_layer (lua_State* tolua_S) {
 return tolua_physics_layer_open(tolua_S);
};
#endif

