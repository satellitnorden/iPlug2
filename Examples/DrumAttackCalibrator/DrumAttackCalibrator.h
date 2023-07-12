#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//Resources.
#include <Resources/Core/SoundResource.h>

//Sound.
#include <Sound/SoundResourcePlayer.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

enum EParams
{
  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class DrumAttackCalibrator final : public Plugin
{

public:

  DrumAttackCalibrator(const InstanceInfo& info);
  ~DrumAttackCalibrator();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

  //The sound resource.
  SoundResource _SoundResource;

  //The sound resource players.
  DynamicArray<SoundResourcePlayer> _SoundResourcePlayers;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

};
