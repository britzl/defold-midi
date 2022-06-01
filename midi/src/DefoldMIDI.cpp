#define EXTENSION_NAME midi
#define LIB_NAME "midi"
#define MODULE_NAME "midi"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif
#include <dmsdk/sdk.h>
#include "DefoldMIDI.h"

#if defined(DM_PLATFORM_HTML5)
#include <emscripten.h>
#endif

#if defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_OSX)

#include "RtMidi.h"
#include <cstdlib>

static RtMidiIn  *g_midiin = 0;
static RtMidiOut *g_midiout = 0;
static dmScript::LuaCallbackInfo* g_Listener = 0;
static MIDIEventQueue *g_EventQueue;
static int g_IsReady = 0;


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
	const char* name = luaL_checkstring(L, 2);
	midi->openPort(port, name);
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

static int IgnoreTypes(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 0);
	int midiSysex = lua_toboolean(L, 1);
	int midiTime = lua_toboolean(L, 2);
	int midiSense = lua_toboolean(L, 3);	
	g_midiin->ignoreTypes(midiSysex, midiTime, midiSense);
	return 0;
}


static int CheckIsReady()
{
#if defined(DM_PLATFORM_HTML5)
	int b = MAIN_THREAD_EM_ASM_INT( {
		return ( typeof window._rtmidi_internals_midi_access !== "undefined" )
	} );
	return b;
#else
	return 1;
#endif
}

static int IsReady(lua_State* L)
{
	DM_LUA_STACK_CHECK(L, 1);
	lua_pushboolean(L, CheckIsReady());
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

static void QueueEvent(MIDIEvent event)
{
	DM_MUTEX_SCOPED_LOCK(g_EventQueue->m_Mutex);
	if(g_EventQueue->m_Events.Full())
	{
		g_EventQueue->m_Events.OffsetCapacity(2);
	}
	g_EventQueue->m_Events.Push(event);
}

static void MidiErrorHandler(RtMidiError::Type type, const std::string &errorText, void *userData)
{
	dmLogError("%s", errorText.c_str());
	MIDIEvent event;
	event.m_Type = MIDI_ERROR;
	event.m_ErrorMessage = strdup(errorText.c_str());
	QueueEvent(event);
}


static void MidiCallback( double timeStamp, std::vector<unsigned char> *message, void *userData )
{
	int nBytes = message->size();
	if (nBytes == 0)
	{
		return;
	}

	dmLogInfo("nBytes %d", nBytes);
	MIDIEvent event;
	event.m_Type = MIDI_MESSAGE;
	event.m_MessageData = new uint8_t[nBytes];
	event.m_MessageDataSize = nBytes;
	for ( int i=nBytes-1; i>=0; i-- )
	{
		event.m_MessageData[i] = message->back();
		message->pop_back();
	}
	QueueEvent(event);
}


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

	{"is_ready", IsReady},
	{"ignore_types", IgnoreTypes},
	{"send_message", SendMessage},
	//{"get_messages", GetMessages},
	{"set_listener", SetListener},
	{0, 0}
};

static void LuaInit(lua_State* L)
{
	int top = lua_gettop(L);

	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name, val) \
	lua_pushnumber(L, (lua_Number) val); \
	lua_setfield(L, -2, #name);\

	SETCONSTANT(EVENT_MESSAGE, MIDI_MESSAGE);
	SETCONSTANT(EVENT_READY,   MIDI_READY);
	SETCONSTANT(EVENT_ERROR,   MIDI_ERROR);
		
	lua_pop(L, 1);
	assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeDefoldMIDI(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDefoldMIDI(dmExtension::Params* params) {
	g_midiin = new RtMidiIn();
	g_midiout = new RtMidiOut();
	g_midiin->setErrorCallback(MidiErrorHandler, 0);
	g_midiout->setErrorCallback(MidiErrorHandler, 0);
	g_midiin->setCallback(MidiCallback, 0);

	g_EventQueue = new MIDIEventQueue();
	g_EventQueue->m_Mutex = dmMutex::New();

	LuaInit(params->m_L);
	dmLogInfo("Registered Defold MIDI Extension %s", MODULE_NAME);
	return dmExtension::RESULT_OK;
}

dmExtension::Result UpdateDefoldMIDI(dmExtension::Params* params)
{
	if (!g_IsReady)
	{
		int ready = CheckIsReady();
		if (ready)
		{
			g_IsReady = 1;
			MIDIEvent event;
			event.m_Type = MIDI_READY;
			QueueEvent(event);
		}
	}

	if (g_EventQueue->m_Events.Empty())
	{
		return dmExtension::RESULT_OK;
	}

	dmArray<MIDIEvent> events;
	{
		DM_MUTEX_SCOPED_LOCK(g_EventQueue->m_Mutex);
		events.Swap(g_EventQueue->m_Events);
	}

	if (!dmScript::IsCallbackValid(g_Listener))
	{
		return dmExtension::RESULT_OK;
	}

	lua_State* L = dmScript::GetCallbackLuaContext(g_Listener);
	DM_LUA_STACK_CHECK(L, 0);

	for(uint32_t i = 0; i != events.Size(); ++i)
	{
		if (!dmScript::SetupCallback(g_Listener))
		{
			return dmExtension::RESULT_OK;
		}

		MIDIEvent *event = &events[i];

		lua_newtable(L);

		lua_pushstring(L, "event");
		lua_pushnumber(L, event->m_Type);
		lua_rawset(L, -3);

		if (event->m_Type == MIDI_MESSAGE)
		{
			lua_pushstring(L, "data");
			lua_newtable(L);
			for ( int i=0; i<event->m_MessageDataSize; i++ )
			{
				lua_pushnumber(L, i + 1);
				lua_pushnumber(L, event->m_MessageData[i]);
				lua_rawset(L, -3);
			}
			lua_rawset(L, -3);

			delete event->m_MessageData;
		}
		else if (event->m_Type == MIDI_ERROR)
		{
			lua_pushstring(L, "error");
			lua_pushstring(L, event->m_ErrorMessage);
			lua_rawset(L, -3);

			delete event->m_ErrorMessage;
		}

		dmScript::PCall(L, 2, 0);
		dmScript::TeardownCallback(g_Listener);
	}

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

	if (g_EventQueue)
	{
		dmMutex::Delete(g_EventQueue->m_Mutex);
		delete g_EventQueue;
		g_EventQueue = 0;
	}

	g_IsReady = 0;

	return dmExtension::RESULT_OK;
}

void OnEventDefoldMIDI(dmExtension::Params* params, const dmExtension::Event* event) { }

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