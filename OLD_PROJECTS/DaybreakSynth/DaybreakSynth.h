#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControl.h"

#include "ADSREnvelope.h"
#include "Oscillator.h"

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

const int kNumPrograms = 1;

enum EParams
{
  PRE_GAIN = 0,
  WAVE_TYPE = 1,
  WHITE_NOISE = 2,
  DISTORTION = 3,
  ATTACK_VALUE = 4,
  DECAY_VALUE = 5,
  SUSTAIN_VALUE = 6,
  RELEASE_VALUE = 7,
  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class PlayingNote final
{

public:

  //The oscillator.
  Oscillator _Oscillator;

  //The ADSR envelope.
  ADSREnvelope _ADSREnvelope;

  //The note number.
  int _NoteNumber;

};

class DaybreakSynth : public Plugin
{

public:

  DaybreakSynth(const InstanceInfo& info);

#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void OnReset() override;
  void OnParamChange(const int index) override;
  void ProcessBlock(sample** inputs, sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

private:

  //The midi queue.
  IMidiQueue _MidiQueue;

  //The playing notes.
  DynamicArray<PlayingNote> _PlayingNotes;

  //The wave type.
  Oscillator::OscillatorMode _WaveType{Oscillator::OscillatorMode::SINE };

};
