#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//Procedural musician.
#include "ProceduralMusicianBand.h"
#include "SectionGenerator.h"

class FourthSectionGenerator : public SectionGenerator
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
        //Send the bass guitar.
        ProceduralMusicianBassGuitar::AddOutgoingMIDIEvent(context, NoteDuration::SIXTEENTH, NoteType::REGULAR);

        //Send the rythm guitars.
        ProceduralMusicianRythmGuitar::AddOutgoingMIDIEvent(context, ProceduralMusicianRythmGuitar::Channel::BOTH, ProceduralMusicianRythmGuitar::Type::OPEN, NoteDuration::SIXTEENTH, NoteType::REGULAR);

        break;
      }
    }
  }

};