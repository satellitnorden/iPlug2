#pragma once

//Shared code.
#include <DrumEngine/DrumEngine.h>

//IPlug.
#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum PLUGIN_PARAMETERS : uint8
{
  CURRENT_SOUNDBANK = 0,
  NUMBER_OF_PLUGIN_PARAMETERS
};

using namespace iplug;
using namespace igraphics;

class DrumEnginePlugin final : public Plugin
{
public:
  DrumEnginePlugin(const InstanceInfo &info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
#endif

private:

  //Friend declarations.
  friend class DropDownMenu;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The drum engine.
  DrumEngine _DrumEngine;

  /*
  * Sets up the "INFINITY" soundbank.
  */
  void SetUpInfinitySoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT;

  /*
  * Sets up the "PANGAEA" soundbank.
  */
  void SetUpPangaeaSoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT;

};
