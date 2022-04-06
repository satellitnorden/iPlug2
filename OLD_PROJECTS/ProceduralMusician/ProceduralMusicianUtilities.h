#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//Sound.
#include <Sound/SoundUtilities.h>

class ProceduralMusicianUtilities final
{

public:

  /*
  * Calculates the note duration in ticks.
  */
  FORCE_INLINE static NO_DISCARD int32 CalculateNoteDurationTicks(  const NoteDuration note_duration,
                                                                    const NoteType note_type,
                                                                    const float64 beats_per_minute,
                                                                    const float64 sample_rate) NOEXCEPT
  {
    const float64 duration_in_seconds{ SoundUtilities::CalculateNoteDuration(note_duration, note_type, beats_per_minute) };

    return static_cast<int32>(duration_in_seconds * sample_rate);
  }

};