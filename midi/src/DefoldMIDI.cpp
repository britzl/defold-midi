#define EXTENSION_NAME DefoldMIDI
#define LIB_NAME "DefoldMIDI"
#define MODULE_NAME "DefoldMIDI"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_HTML5)

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