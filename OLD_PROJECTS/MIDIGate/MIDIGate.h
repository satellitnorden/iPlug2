#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/StaticArray.h>

#define USE_OUTPUT_LOG (0)

enum EParams
{
  NUMBER_OF_PARAMS
};

class MIDIGate final : public iplug::Plugin
{

public:


  MIDIGate(const iplug::InstanceInfo& info);
  ~MIDIGate();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The current gate value.
  int32 _CurrentGateValue{ 0 };

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The number of MIDI notes playing.
  int32 _NumberOfMIDINotesPlaying{ 0 };

  //The buffer.
  StaticArray<StaticArray<iplug::sample, MIDI_GATE_BUFFER_SIZE>, 2> _Buffer;

  //The current buffer index.
  uint64 _CurrentBufferIndex{ 0 };

  /*
  * Returns the prevoius buffer value.
  */
  FORCE_INLINE NO_DISCARD iplug::sample GetPreviousBufferValue(const uint8 channel_index) NOEXCEPT
  {
    return _Buffer[channel_index][(_CurrentBufferIndex + 1) & (MIDI_GATE_BUFFER_SIZE - 1)];
  }

  /*
  * Sets the current buffer value.
  */
  FORCE_INLINE void SetCurrentBufferValue(const uint8 channel_index, const iplug::sample value) NOEXCEPT
  {
    _Buffer[channel_index][_CurrentBufferIndex] = value;
  }

};
