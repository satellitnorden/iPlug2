#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

enum EParams
{
  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class DrumAttackCompanion final : public Plugin
{

public:

  DrumAttackCompanion(const InstanceInfo& info);
  ~DrumAttackCompanion();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The current channel.
  int32 _CurrentChannel{ 0 };

  //The texts.
  DynamicArray<iplug::igraphics::ITextControl *const RESTRICT> _Texts;

};
