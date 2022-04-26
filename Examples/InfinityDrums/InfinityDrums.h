#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/StaticArray.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Math.
#include <Math/General/RandomIndexer.h>

//Sound.
#include <Sound/ADSREnvelope.h>
#include <Sound/SoundResourcePlayer.h>

enum class DrumPart : uint8
{
  KICK,
  SNARE,
  SNARE_SIDE_STICK,
  TOM_1,
  TOM_2,
  TOM_3,
  TOM_4,
  CHINA,
  RIDE_BOW,
  RIDE_BELL,
  RIDE_EDGE,
  RIGHT_CRASH,
  SPLASH,
  LEFT_CRASH,
  HIHAT_PEDAL,
  HIHAT_FULLY_CLOSED,
  HIHAT_LEVEL_0,
  HIHAT_LEVEL_1,
  HIHAT_LEVEL_2,
  HIHAT_LEVEL_3,
  HIHAT_LEVEL_4,
  HIHAT_FULLY_OPEN,
  LEFT_WIDE_CRASH,
  STACK,

  NUMBER_OF_DRUM_PARTS,

  NONE
};

// Alesis drums constants.
namespace AlesisDrumsConstants
{
  constexpr uint8 NUMBER_OF_LAYERS{ 10 };
}

class DrumPartProperties final
{

public:

  //Enumeration covering all velocity curves.
  enum class VelocityCurve : uint8
  {
    LINEAR,
    HALF_INVERSE_SQUARE,
    INVERSE_SQUARE,
    MAXIMUM
  };

  //The drum part string.
  const char* RESTRICT _DrumPartString;

  //The sound resources.
  DynamicArray<StaticArray<SoundResource, AlesisDrumsConstants::NUMBER_OF_LAYERS>> _SoundResources;

  //The random indexer.
  RandomIndexer<AlesisDrumsConstants::NUMBER_OF_LAYERS> _RandomIndexer;

  //The midi number.
  uint32 _MidiNumber;

  //The velocity curve.
  VelocityCurve _VelocityCurve;

  //The pan.
  float32 _Pan;

  //The gain.
  float32 _Gain;

};

class PlayingNote final
{

public:

  //The sound resource player.
  SoundResourcePlayer _SoundResourcePlayer;

  //The drum part.
  DrumPart _DrumPart;

};

using namespace iplug;
using namespace igraphics;

class InfinityDrums final : public Plugin
{
public:
  InfinityDrums(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void ProcessBlock(sample **inputs, sample **outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The text. (:
  iplug::igraphics::ITextControl *RESTRICT _Text{ nullptr };

  //The drum part properties.
  StaticArray<DrumPartProperties, UNDERLYING(DrumPart::NUMBER_OF_DRUM_PARTS)> _DrumPartProperties;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  /*
  * Exports packages.
  */
  void ExportPackages() NOEXCEPT;

  /*
  * Imports packages.
  */
  void ImportPackages() NOEXCEPT;

};
