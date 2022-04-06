#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Core.
#include <Core/Essential/CatalystEssential.h>

using namespace iplug;
using namespace igraphics;

class AutoGainStager final : public Plugin
{
public:
  AutoGainStager(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

  bool SerializeState(iplug::IByteChunk& chunk) const override;
  int32 UnserializeState(const iplug::IByteChunk& chunk, int32 startPos) override;
};
