#if defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_OSX)

#ifndef DEFOLD_MIDI_H
#define DEFOLD_MIDI_H

#include <dmsdk/sdk.h>


enum EMIDIEvent
{
    MIDI_MESSAGE,
    MIDI_READY,
    MIDI_ERROR
};

struct DM_ALIGNED(16) MIDIEvent
{
    MIDIEvent()
    {
        memset(this, 0, sizeof(MIDIEvent));
    }
     
    uint32_t            m_Type;
    uint32_t            m_MessageDataSize;
    uint8_t*            m_MessageData;
    const char*         m_ErrorMessage;
};

struct MIDIEventQueue
{
    dmArray<MIDIEvent> m_Events;
    dmMutex::HMutex    m_Mutex;
};

#endif

#endif