#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

//iPlug.
#include "IPlug_include_in_plug_hdr.h"

//Using specifications.
using namespace iplug;
using namespace igraphics;

class ProceduralDrummer final : public Plugin
{

public:

  /*
  * Default constructor.
  */
  ProceduralDrummer(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void ProcessBlock(sample **inputs, sample **outputs, int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg &message) override;
#endif

private:

  //The sample rate.
  uint64 _SampleRate;

  //The incoming midi queue.
  iplug::IMidiQueue _IncomingMidiQueue;

};
