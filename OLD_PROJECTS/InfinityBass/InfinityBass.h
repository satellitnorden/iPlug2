#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//Concurrency.
#undef Yield;
#include <Concurrency/AtomicFlag.h>

//Math.
#include <Math/General/RandomIndexer.h>

//Sound.
#include <Sound/ADSREnvelope.h>
#include <Sound/SoundResourcePlayer.h>

//Infinity guitar.
#include "InfinityBassNote.h"

#define USE_OUTPUT_LOG (0)

enum EParams
{
  HUMANIZE_PARAM,

  NUMBER_OF_PARAMS
};

class PlayingNote final
{

public:

  //The sound resource player.
  SoundResourcePlayer _SoundResourcePlayer;

  //The note number.
  int32 _NoteNumber;

};


class InfinityBass final : public iplug::Plugin
{
public:

  InfinityBass(const iplug::InstanceInfo& info);
  ~InfinityBass();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The humanize knob.
  iplug::igraphics::IVKnobControl *RESTRICT _HumanizeKnob{ nullptr };

  //The notes.
  DynamicArray<InfinityBassNote> _Notes;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  //The humanize factor.
  float32 _HumanizeFactor{ 0.0f };

  //The random indexer.
  RandomIndexer<4> _RandomIndexer;

#if USE_OUTPUT_LOG
  //The output log.
  std::ofstream _OutputLog;
#endif

  /*
  * Exports the packages.
  */
  void ExportPackages() NOEXCEPT;

  /*
  * Imports the packages.
  */
  void ImportPackages() NOEXCEPT;

};
