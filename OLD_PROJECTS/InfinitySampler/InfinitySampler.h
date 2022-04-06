#pragma once

//Core.
#define CATALYST_PLATFORM_WINDOWS
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Sound.
#include <Sound/SoundResourcePlayer.h>

//Infinity sampler.
#include "InfinitySamplerSaveState.h"

enum EParams
{
  FOLDER,
  MIDI_NOTE,
  VELOCITY_CURVE,
  HUMANIZE,
  ATTACK_VALUE,
  DECAY_VALUE,
  SUSTAIN_VALUE,
  RELEASE_VALUE,
  MAX_SAMPLES,
  NUMBER_OF_PARAMS
};

class PlayingNote final
{

public:

  //The sound resource player.
  SoundResourcePlayer _SoundResourcePlayer;

};

using namespace iplug;
using namespace igraphics;

class InfinitySampler final : public Plugin
{
public:
  InfinitySampler(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

  bool SerializeState(iplug::IByteChunk& chunk) const override;
  int32 UnserializeState(const iplug::IByteChunk& chunk, int32 startPos) override;

private:

  //The text. (:
  iplug::igraphics::ITextControl *RESTRICT _Text{ nullptr };

  //The sound resources.
  DynamicArray<SoundResource> _SoundResources;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  //The humanize factor.
  float _HumanizeFactor{ 0.0f };

  //The last played note index.
  uint64 _LastPlayedNoteIndex{ 0 };

  //The save state.
  InfinitySamplerSaveState _SaveState;

  /*
  * Selects the file.
  */
  void SelectFile() NOEXCEPT;

  /*
  * Selects the folder.
  */
  void SelectFolder() NOEXCEPT;

  /*
  * Reads the sound resource.
  */
  void ReadSoundResource(const char *const RESTRICT file) NOEXCEPT;

  /*
  * Reads all the sound resources.
  */
  void ReadSoundResources(const char *const RESTRICT folder) NOEXCEPT;

  /*
  * Selects the next index to play.
  */
  uint64 SelectNextIndex(const uint64 maximum, const uint64 last) NOEXCEPT;

};
