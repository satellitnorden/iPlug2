#pragma once

//Shared code.
#include <DrumEngine/DrumEngine.h>

//IPlug.
#include "IPlug_include_in_plug_hdr.h"

class DrumEnginePluginSaveData final
{

public:

  //The current version.
  constexpr static uint64 CURRENT_VERSION{ 1 };

  //The version.
  uint64 _Version;

  //The preset file path.
  StaticString<MAXIMUM_FILE_PATH_LENGTH> _PresetFilePath;

  //The preset.
  DrumEnginePreset _Preset;

};

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

  void OnIdle() override;

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
#endif

  bool SerializeState(iplug::IByteChunk &chunk) const override;
  int UnserializeState(const iplug::IByteChunk &chunk, int start_position) override;

private:

  //Friend declarations.
  friend class DrumEnginePluginTextControl;
  friend class DrumEnginePluginButtonControl;
  friend class DrumEnginePluginDropDownMenuControl;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The drum engine.
  DrumEngine _DrumEngine;

  //The save data.
  DrumEnginePluginSaveData _SaveData;

  //The controls.
  DynamicArray<class DrumEnginePluginControl *RESTRICT> _Controls;

  //Denotes if UI needs to be rebuilt.
  bool _RebuildUI{ false };

  //The start mixer channel index.
  uint32 _StartMixerChannelIndex{ 0 };

  /*
  * Sets up the "INFINITY" soundbank.
  */
  void SetUpInfinitySoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT;

  /*
  * Sets up the "PANGAEA" soundbank.
  */
  void SetUpPangaeaSoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT;

  /*
  * Sets up the "SINGULARITY" soundbank.
  */
  void SetUpSingularitySoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT;

};
