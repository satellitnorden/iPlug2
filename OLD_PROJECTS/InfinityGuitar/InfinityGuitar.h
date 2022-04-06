#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Concurrency.
#undef Yield;
#define CATALYST_PLATFORM_WINDOWS
#include <Concurrency/AtomicFlag.h>

//Math.
#include <Math/General/RandomIndexer.h>

//Sound.
#include <Sound/ADSREnvelope.h>
#include <Sound/SoundResourcePlayer.h>

//Infinity guitar.
#include "InfinityGuitarNote.h"

#define USE_OUTPUT_LOG (0)

enum EParams
{
  CHANNEL_PARAM,
  TYPE_PARAM,
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

using namespace iplug;
using namespace igraphics;

class InfinityGuitar final : public Plugin
{
public:

  //Enumeration covering all channels.
  enum class Channel : uint8
  {
    NONE,
    LEFT,
    RIGHT
  };

  //Enumeration covering all types.
  enum class Type : uint8
  {
    NONE,
    DI,
    RHYTHM,
    LEAD,
    CLEAN
  };

  InfinityGuitar(const iplug::InstanceInfo& info);
  ~InfinityGuitar();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The channel knob.
  iplug::igraphics::IVKnobControl *RESTRICT _ChannelKnob{ nullptr };

  //The type knob.
  iplug::igraphics::IVKnobControl *RESTRICT _TypeKnob{ nullptr };

  //The humanize knob.
  iplug::igraphics::IVKnobControl *RESTRICT _HumanizeKnob{ nullptr };

  //The notes.
  DynamicArray<InfinityGuitarNote> _Notes;

  //The dead notes.
  StaticArray<StaticArray<SoundResource, 4>, 8> _DeadNotes;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  //The current channel.
  Channel _CurrentChannel{ Channel::NONE };

  //The current type.
  Type _CurrentType{ Type::NONE };

  //The humanize factor.
  float _HumanizeFactor{ 0.0f };

  //The random indexer.
  RandomIndexer<4> _RandomIndexer;

  //The dead note random indexer.
  RandomIndexer<4> _DeadNoteRandomIndexer;

  //Denotes whether or not the plugin wants to reload notes.
  AtomicFlag _WantsToReloadNotes;

  //Denotes whether or not the plugin is ready to reload notes.
  AtomicFlag _IsReadyToToReloadNotes;

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
  void ImportPackages(const Channel channel, const Type type) NOEXCEPT;

};
