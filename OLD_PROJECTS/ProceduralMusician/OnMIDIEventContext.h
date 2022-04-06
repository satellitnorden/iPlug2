#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

class OnMIDIEventContext final
{

public:

  //The beats per minute.
  float64 _BeatsPerMinute;

  //The sample rate.
  float64 _SampleRate;

  //The note number.
  uint8 _NoteNumber;

  //The MIDI events.
  DynamicArray<OutgoingMIDIEvent> *_MIDIEvents;

};