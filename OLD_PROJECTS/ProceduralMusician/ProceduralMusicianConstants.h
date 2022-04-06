#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

namespace ProceduralMusicianConstants
{
  constexpr int32 NOTE_OFF_TICK_COMPENSATION{ 1 };

  constexpr int32 KICK_NOTE_NUMBER{ 24 };

  constexpr int32 BASS_CHANNEL{ 1 }; //Channel 2 in DAW's.
  constexpr int32 LEFT_RYTHM_GUITAR_CHANNEL{ 2 }; //Channel 3 in DAW's.
  constexpr int32 RIGHT_RYTHM_GUITAR_CHANNEL{ 3 }; //Channel 4 in DAW's.
}