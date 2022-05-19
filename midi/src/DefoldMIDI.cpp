#define LIB_NAME "DefoldMIDI"
#define MODULE_NAME "midi"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN "midi"
#endif
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_OSX) || defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_LINUX) || defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_HTML5)

#include "RtMidi.h"

dmExtension::Result AppInitializeDefoldMIDI(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDefoldMIDI(dmExtension::Params* params) {	
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

#else

dmExtension::Result AppInitializeDefoldMIDI(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDefoldMIDI(dmExtension::Params* params) {	
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

#endif

DM_DECLARE_EXTENSION(DefoldMIDI, LIB_NAME, AppInitializeDefoldMIDI, 0, InitializeDefoldMIDI, UpdateDefoldMIDI, OnEventDefoldMIDI, FinalizeDefoldMIDI)
