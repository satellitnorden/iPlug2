#pragma once

#include "IPlug_include_in_plug_hdr.h"

using namespace iplug;
using namespace igraphics;

class DaybreakPanInvert final : public Plugin
{
public:
  DaybreakPanInvert(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
#endif
};
