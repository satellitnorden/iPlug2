#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//Procedural Musician.
#include "OnMIDIEventContext.h"
#include "ProceduralMusicianConstants.h"
#include "ProceduralMusicianUtilities.h"

class ProceduralMusicianRythmGuitar final
{

public:

  //Enumeration covering all channels.
  enum class Channel : uint8
  {
    LEFT,
    RIGHT,
    BOTH
  };

  //Enumeration covering all types.
  enum class Type : uint8
  {
    OPEN,
    MUTED,
    TAPPED
  };

  /*
  * Adds an outgoing bass guitar MIDI event.
  */
  FORCE_INLINE static void AddOutgoingMIDIEvent(const OnMIDIEventContext &context, const Channel channel, const Type type, const NoteDuration note_duration, const NoteType note_type) NOEXCEPT
  {
    //Calculate the velocity.
    int32 velocity;

    switch (type)
    {
      case Type::OPEN:
      {
        velocity = 127;

        break;
      }

      case Type::MUTED:
      {
        velocity = 90;

        break;
      }

      case Type::TAPPED:
      {
        velocity = 80;

        break;
      }

      default:
      {
        ASSERT(false, "Invalid case!");

        break;
      }
    }

    //Add the left rythm guitar MIDI event.
    if (channel == Channel::LEFT
        || channel == Channel::BOTH)
    {
      //Remove any other note off messages present on this note from the outgoing MIDI queue.
      for (uint64 i{ 0 }; i < context._MIDIEvents->Size();)
      {
        if (context._MIDIEvents->At(i)._Message.Channel() == ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL
            && context._MIDIEvents->At(i)._Message.StatusMsg() == iplug::IMidiMsg::EStatusMsg::kNoteOff
            && context._MIDIEvents->At(i)._Message.NoteNumber() == 40)
        {
          context._MIDIEvents->EraseAt<true>(i);
        }

        else
        {
          ++i;
        }
      }

      //Add the first note off MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, 0);
      }

      //Add the note on MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOnMsg(40, velocity, 0, ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, 0);
      }

      //Add the second note off MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, ProceduralMusicianUtilities::CalculateNoteDurationTicks(NoteDuration::SIXTEENTH, NoteType::REGULAR, context._BeatsPerMinute, context._SampleRate) - ProceduralMusicianConstants::NOTE_OFF_TICK_COMPENSATION);
      }
    }

    //Add the right rythm guitar MIDI event.
    if (channel == Channel::RIGHT
        || channel == Channel::BOTH)
    {
      //Remove any other note off messages present on this note from the outgoing MIDI queue.
      for (uint64 i{ 0 }; i < context._MIDIEvents->Size();)
      {
        if (context._MIDIEvents->At(i)._Message.Channel() == ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL
          && context._MIDIEvents->At(i)._Message.StatusMsg() == iplug::IMidiMsg::EStatusMsg::kNoteOff
          && context._MIDIEvents->At(i)._Message.NoteNumber() == 40)
        {
          context._MIDIEvents->EraseAt<true>(i);
        }

        else
        {
          ++i;
        }
      }

      //Add the first note off MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, 0);
      }

      //Add the note on MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOnMsg(40, velocity, 0, ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, 0);
      }

      //Add the second note off MIDI event.
      {
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL);

        context._MIDIEvents->Emplace(output_message, ProceduralMusicianUtilities::CalculateNoteDurationTicks(NoteDuration::SIXTEENTH, NoteType::REGULAR, context._BeatsPerMinute, context._SampleRate) - ProceduralMusicianConstants::NOTE_OFF_TICK_COMPENSATION);
      }
    }
  }

};