#define EXTENSION_NAME midi
#define LIB_NAME "midi"
#define MODULE_NAME "midi"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_HTML5)

#include "RtMidi.h"

static int Test(lua_State* L)
{
	//
	printf("test\n");
	return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
	{"test", Test},
	{0, 0}
};

static void LuaInit(lua_State* L)
{
	int top = lua_gettop(L);

	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);

	lua_pop(L, 1);
	assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeDefoldMIDI(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDefoldMIDI(dmExtension::Params* params) {
	LuaInit(params->m_L);
	dmLogInfo("Registered Defold MIDI Extension", MODULE_NAME);
	return dmExtension::RESULT_OK;
}

dmExtension::Result UpdateDefoldMIDI(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDefoldMIDI(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

void OnEventDefoldMIDI(dmExtension::Params* params, const dmExtension::Event* event) {
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeDefoldMIDI, 0, InitializeDefoldMIDI, UpdateDefoldMIDI, OnEventDefoldMIDI, FinalizeDefoldMIDI)

#else

dmExtension::Result InitializeDefoldMIDI(dmExtension::Params* params) {	
	return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDefoldMIDI(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, 0, 0, InitializeDefoldMIDI, 0, 0, FinalizeDefoldMIDI)

#endif