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
static dmScript::LuaCallbackInfo* g_Listener = 0;


static int Count(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 1);
	int count = midi->getPortCount();
	lua_pushnumber(L, count);
	return 1;
}


static int Open(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 0);
	int port = luaL_checknumber(L, 1);
	if (lua_isstring(L, 2))
	{
		const char* name = luaL_checkstring(L, 2);
		midi->openPort(port, name);
	}
	else
	{
		midi->openPort(port);
	}
	return 0;
}

static int OpenVirtual(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 0);
	const char* name = luaL_checkstring(L, 1);
	midi->openVirtualPort(name);
	return 0;

}

static int Close(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 0);
	midi->closePort();
	return 0;
}

static int IsOpen(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 1);
	int open = midi->isPortOpen();
	lua_pushboolean(L, open);
	return 1;

}

static int GetMessages(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 1);
	std::vector<unsigned char> message;
	double stamp = g_midiin->getMessage(&message);
	int nBytes = message.size();
	lua_newtable(L);
	for ( int i=0; i<nBytes; i++ )
	{
		lua_pushnumber(L, (int)message[i]);
		lua_rawseti(L, -2, 1);
	}
	return 1;
}

static int SendMessage(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);

 	std::vector<unsigned char> message;
	while (true) {
		int index = message.size();
		lua_pushinteger(L, index + 1);
		lua_gettable(L, -2);
		if (lua_type(L, -1) == LUA_TNIL)
		{
			break;
		}
		lua_Number v = luaL_checknumber(L, -1);
		message.push_back(v);
		lua_pop(L, 1);
	}

	g_midiout->sendMessage(&message);
	return 0;
}

static int SetListener(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	if (g_Listener)
	{
		dmScript::DestroyCallback(g_Listener);
		g_Listener = 0;
	}
	g_Listener = dmScript::CreateCallback(L, 1);
	return 0;
}

static int GetPortName(lua_State* L, RtMidi* midi)
{
	DM_LUA_STACK_CHECK(L, 1);
	int port = luaL_checknumber(L, 1);
	const char* name = midi->getPortName(port).c_str();
	lua_pushstring(L, name);
	return 1;
}


static int CountOut(lua_State* L) { return Count(L, g_midiout); }
static int CountIn(lua_State* L) { return Count(L, g_midiin); }

static int OpenOut(lua_State* L) { return Open(L, g_midiout); }
static int OpenIn(lua_State* L) { return Open(L, g_midiin); }

static int OpenVirtualOut(lua_State* L) { return OpenVirtual(L, g_midiout); }
static int OpenVirtualIn(lua_State* L) { return OpenVirtual(L, g_midiin); }

static int CloseOut(lua_State* L) { return Close(L, g_midiout); }
static int CloseIn(lua_State* L) { return Close(L, g_midiin); }

static int IsOutOpen(lua_State* L)  { return IsOpen(L, g_midiout); }
static int IsInOpen(lua_State* L)  { return IsOpen(L, g_midiin); }

static int GetPortNameOut(lua_State* L) { return GetPortName(L, g_midiout); }
static int GetPortNameIn(lua_State* L) { return GetPortName(L, g_midiin); }


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
	{"is_out_open", IsOutOpen},
	{"is_in_open", IsInOpen},
	{"get_name_out", GetPortNameOut},
	{"get_name_in", GetPortNameIn},

	{"send_message", SendMessage},
	{"get_messages", GetMessages},
	{"set_listener", SetListener},
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
	std::vector<unsigned char> message;
	double stamp = g_midiin->getMessage( &message );
	int nBytes = message.size();
	if (nBytes == 0)
	{
		return dmExtension::RESULT_OK;
	}

	if (!dmScript::IsCallbackValid(g_Listener))
	{
		return dmExtension::RESULT_OK;
	}

	lua_State* L = dmScript::GetCallbackLuaContext(g_Listener);
	DM_LUA_STACK_CHECK(L, 0);


	if (!dmScript::SetupCallback(g_Listener))
	{
		return dmExtension::RESULT_OK;
	}

	lua_newtable(L);
	for ( int i=0; i<nBytes; i++ )
	{
		lua_pushnumber(L, i + 1);
		lua_pushnumber(L, (int)message[i]);
		lua_rawset(L, -3);
	}

	dmScript::PCall(L, 2, 0);

	dmScript::TeardownCallback(g_Listener);

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

	if (g_Listener)
	{
		dmScript::DestroyCallback(g_Listener);
		g_Listener = 0;
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