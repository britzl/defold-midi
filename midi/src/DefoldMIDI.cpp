#define EXTENSION_NAME midi
#define LIB_NAME "midi"
#define MODULE_NAME "midi"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_OSX)

#include "RtMidi.h"
#include <cstdlib>

static RtMidiIn  *g_midiin = 0;
static RtMidiOut *g_midiout = 0;

static int CountOut(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 1);
	int count = g_midiout->getPortCount();
	lua_pushnumber(L, count);
	return 1;
}

static int CountIn(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 1);
	int count = g_midiin->getPortCount();
	lua_pushnumber(L, count);
	return 1;
}

static int OpenOut(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	int port = luaL_checknumber(L, 1);
	g_midiout->openPort(port);
	return 0;
}

static int OpenIn(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	int port = luaL_checknumber(L, 1);
	g_midiin->openPort(port);
	return 0;
}

static int OpenVirtualOut(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	const char* name = luaL_checkstring(L, 1);
	g_midiout->openVirtualPort(name);
	return 0;
}

static int OpenVirtualIn(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	const char* name = luaL_checkstring(L, 1);
	g_midiin->openVirtualPort(name);
	return 0;
}

static int CloseOut(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	g_midiout->closePort();
	return 0;
}

static int CloseIn(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	g_midiin->closePort();
	return 0;
}

static int GetMessages(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 1);
	std::vector<unsigned char> message;
	double stamp = g_midiin->getMessage( &message );
	int nBytes = message.size();
	lua_newtable(L);
	for ( int i=0; i<nBytes; i++ )
	{
		lua_pushnumber(L, (int)message[i]);
		lua_rawseti(L, -2, 1);
	}
	return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
	{"count_out", CountOut},
	{"count_in", CountIn},
	{"open_out", OpenOut},
	{"open_in", OpenIn},
	{"open_virtual_out", OpenVirtualOut},
	{"open_virtual_in", OpenVirtualIn},
	{"close_out", CloseOut},
	{"close_in", CloseIn},
	{"get_messages", GetMessages},
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
	g_midiin = new RtMidiIn();
	g_midiout = new RtMidiOut();
	
	LuaInit(params->m_L);
	dmLogInfo("Registered Defold MIDI Extension %s", MODULE_NAME);
	return dmExtension::RESULT_OK;
}

dmExtension::Result UpdateDefoldMIDI(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDefoldMIDI(dmExtension::Params* params) {
	if (g_midiin)
	{
		delete g_midiin;
		g_midiin = 0;
	}
	if (g_midiout)
	{
		delete g_midiout;
		g_midiout = 0;
	}
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