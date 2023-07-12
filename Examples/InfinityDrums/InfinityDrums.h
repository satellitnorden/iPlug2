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
  HIHAT_SPLASH,
  HIHAT_FULLY_CLOSED,
  HIHAT_LEVEL_0,
  HIHAT_LEVEL_1,
  HIHAT_LEVEL_2,
  HIHAT_LEVEL_3,
  HIHAT_LEVEL_4,
  HIHAT_LEVEL_5,
  HIHAT_FULLY_OPEN,
  WIDE_LEFT_CRASH,
  STACK,

  NUMBER_OF_DRUM_PARTS,

  NONE
};

//Infinity drums constants.
namespace InfinityDrumsConstants
{
  constexpr uint8 NUMBER_OF_LAYERS{ 12 };
}

class DrumPartProperties final
{

public:

  //The drum part string.
  const char* RESTRICT _DrumPartString;

  //The sound resources.
  DynamicArray<StaticArray<SoundResource, InfinityDrumsConstants::NUMBER_OF_LAYERS>> _SoundResources;

  //The random indexer.
  RandomIndexer<InfinityDrumsConstants::NUMBER_OF_LAYERS> _RandomIndexer;

  //The midi number.
  uint32 _MidiNumber;

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

  //The current hihat CC value.
  float32 _CurrentHihatCCValue{ 0.0f };

  /*
  * Exports packages.
  */
  void ExportPackages() NOEXCEPT;

  /*
  * Imports packages.
  */
  void ImportPackages() NOEXCEPT;

};
