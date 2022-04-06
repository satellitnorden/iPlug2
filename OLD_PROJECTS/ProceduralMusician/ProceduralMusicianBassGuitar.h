#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//Procedural Musician.
#include "OnMIDIEventContext.h"
#include "ProceduralMusicianConstants.h"
#include "ProceduralMusicianUtilities.h"

class ProceduralMusicianBassGuitar final
{

public:

  /*
  * Adds an outgoing bass guitar MIDI event.
  */
  FORCE_INLINE static void AddOutgoingMIDIEvent(const OnMIDIEventContext &context, const NoteDuration note_duration, const NoteType note_type) NOEXCEPT
  {
    //Remove any other note off messages present on this note from the outgoing MIDI queue.
    for (uint64 i{ 0 }; i < context._MIDIEvents->Size();)
    {
      if (context._MIDIEvents->At(i)._Message.Channel() == ProceduralMusicianConstants::BASS_CHANNEL
        && context._MIDIEvents->At(i)._Message.StatusMsg() == iplug::IMidiMsg::EStatusMsg::kNoteOff
        && context._MIDIEvents->At(i)._Message.NoteNumber() == 28)
      {
        context._MIDIEvents->EraseAt<true>(i);
      }

      else
      {
        ++i;
      }
    }

    //Add the first note off message.
    {
      iplug::IMidiMsg output_message;
      output_message.MakeNoteOffMsg(28, 0, ProceduralMusicianConstants::BASS_CHANNEL);

      context._MIDIEvents->Emplace(output_message, 0);
    }

    //Add the note on message.
    {
      iplug::IMidiMsg output_message;
      output_message.MakeNoteOnMsg(28, 127, 0, ProceduralMusicianConstants::BASS_CHANNEL);

      context._MIDIEvents->Emplace(output_message, 0);
    }

    //Add the second note off message.
    {
      iplug::IMidiMsg output_message;
      output_message.MakeNoteOffMsg(28, 0, ProceduralMusicianConstants::BASS_CHANNEL);

      context._MIDIEvents->Emplace(output_message, ProceduralMusicianUtilities::CalculateNoteDurationTicks(NoteDuration::SIXTEENTH, NoteType::REGULAR, context._BeatsPerMinute, context._SampleRate) - ProceduralMusicianConstants::NOTE_OFF_TICK_COMPENSATION);
    }
  }

};