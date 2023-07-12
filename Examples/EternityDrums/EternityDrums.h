#pragma once

//Shared code.
#include <DrumEngine/DrumEngine.h>

//IPlug.
#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class EternityDrums final : public Plugin
{
public:
  EternityDrums(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

private:

  //The drum engine.
  DrumEngine _DrumEngine;

};
