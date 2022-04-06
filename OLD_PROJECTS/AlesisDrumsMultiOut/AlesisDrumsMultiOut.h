#pragma once

//Core.
#define CATALYST_PLATFORM_WINDOWS
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
  HIHAT_HALFWAY,
  HIHAT_FULLY_OPEN,
  LEFT_WIDE_CRASH,
  STACK,
  KICK_ROOM,
  SNARE_ROOM,
  TOM_1_ROOM,
  TOM_2_ROOM,
  TOM_3_ROOM,
  TOM_4_ROOM,

  NUMBER_OF_DRUM_PARTS,

  NONE
};

//Alesis drums constants.
namespace AlesisDrumsConstants
{
  constexpr uint8 NUMBER_OF_LAYERS{ 9 };
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
  const char *RESTRICT _DrumPartString;

  //The sound resources.
  DynamicArray<StaticArray<SoundResource, AlesisDrumsConstants::NUMBER_OF_LAYERS>> _SoundResources;

  //The random indexer.
  RandomIndexer<AlesisDrumsConstants::NUMBER_OF_LAYERS> _RandomIndexer;

  //The midi number.
  uint32 _MidiNumber;

  //The velocity curve.
  VelocityCurve _VelocityCurve;

  //The retrigger cancle time, in seconds.
  float32 _RetriggerCancelTime;

  //The pan.
  float32 _Pan;

  //The gain.
  float32 _Gain;

  //The time since last trigger.
  float32 _TimeSinceLastTrigger{ 0.0f };

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

class AlesisDrumsMultiOut final : public Plugin
{
public:
  AlesisDrumsMultiOut(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
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
