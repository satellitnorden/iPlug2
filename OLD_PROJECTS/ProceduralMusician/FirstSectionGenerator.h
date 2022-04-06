#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//Procedural musician.
#include "ProceduralMusicianBand.h"
#include "SectionGenerator.h"

class FirstSectionGenerator : public SectionGenerator
{

public:

  /*
  * The on MIDI event function.
  */
  FORCE_INLINE void OnMIDIEvent(const OnMIDIEventContext &context) NOEXCEPT override
  {
    switch (context._NoteNumber)
    {
      case ProceduralMusicianConstants::KICK_NOTE_NUMBER:
      {
        //Send the bass message.
        {
          //Add the note off message.
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
        }

        //Send the left rythm guitar message.
        {
          //Add the note off message.
          {
            iplug::IMidiMsg output_message;
            output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL);

            context._MIDIEvents->Emplace(output_message, 0);
          }

          //Add the note on message.
          {
            iplug::IMidiMsg output_message;
            output_message.MakeNoteOnMsg(40, 90, 0, ProceduralMusicianConstants::LEFT_RYTHM_GUITAR_CHANNEL);

            context._MIDIEvents->Emplace(output_message, 0);
          }
        }

        //Send the right rythm guitar message.
        {
          //Add the note off message.
          {
            iplug::IMidiMsg output_message;
            output_message.MakeNoteOffMsg(40, 0, ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL);

            context._MIDIEvents->Emplace(output_message, 0);
          }

          //Add the note on message.
          {
            iplug::IMidiMsg output_message;
            output_message.MakeNoteOnMsg(40, 90, 0, ProceduralMusicianConstants::RIGHT_RYTHM_GUITAR_CHANNEL);

            context._MIDIEvents->Emplace(output_message, 0);
          }
        }

        break;
      }
    }
  }

};