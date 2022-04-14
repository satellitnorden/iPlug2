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
  TRACK_PARAM,
  CHANNEL_PARAM,
  HUMANIZE_PARAM,
  RELEASE_PARAM,

  NUMBER_OF_PARAMS
};

class PlayingNote final
{

public:
  // The sound resource player.
  SoundResourcePlayer _SoundResourcePlayer;

  // The note number.
  int32 _NoteNumber;
};

using namespace iplug;
using namespace igraphics;

class InfinityGuitar final : public Plugin
{

public:

  // Enumeration covering all tracks.
  enum class Track : uint8
  {
    NONE,
    LEFT,
    RIGHT
  };

  // Enumeration covering all channels.
  enum class Channel : uint8
  {
    NONE,
    DI,
    RHYTHM,
    LEAD,
    CLEAN
  };

  InfinityGuitar(const InstanceInfo& info);
  ~InfinityGuitar();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(int index) override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

private:

  //Enumeration covering all loading states.
  enum class LoadingState : uint8
  {
    IDLE,
    LOADING
  };

  //Enumeration covering all errors.
  enum class Error : uint8
  {
    NONE,
    COULDNT_LOAD_SAMPLES_DIRECTORY_DOESNT_EXIST,
    COULDNT_LOAD_SAMPLES_PACKAGE_FILE_OPEN_FAILED
  };

  //The current loading state.
  LoadingState _CurrentLoadingState{ LoadingState::IDLE };

  //The notes.
  DynamicArray<InfinityGuitarNote> _Notes;

  //The dead notes.
  StaticArray<StaticArray<SoundResource, 4>, 8> _DeadNotes;

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  //The current track.
  Track _CurrentTrack{ Track::NONE };

  //The wanted track.
  Track _WantedTrack{Track::NONE};

  //The loaded track.
  Track _LoadedTrack{ Track::NONE };

  //The current channel.
  Channel _CurrentChannel{ Channel::NONE };

  // The wanted channel.
  Channel _WantedChannel{Channel::NONE};

  //The loaded channel.
  Channel _LoadedChannel{ Channel::NONE };

  //The humanize factor.
  float _HumanizeFactor{ 0.0f };

  //The release.
  float _Release{ 0.01f };

  //The random indexer.
  RandomIndexer<4> _RandomIndexer;

  //The dead note random indexer.
  RandomIndexer<4> _DeadNoteRandomIndexer;

  //Denotes whether or not loading samples has finished.
  AtomicFlag _HasFinishedLoadingSamples;

  //The async loading thread.
  std::thread *RESTRICT _AsyncLoadingThread{ nullptr };

  //The error.
  Error _Error{ Error::NONE };

#if USE_OUTPUT_LOG
  //The output log.
  std::ofstream _OutputLog;
#endif

  /*
  * Exports the packages.
  */
  void ExportPackages() NOEXCEPT;

  /*
  * Async imports packages.
  */
  void AsyncImportPackages() NOEXCEPT;

  /*
  * Imports the packages.
  */
  void ImportPackages(const Track track, const Channel channel) NOEXCEPT;

};
