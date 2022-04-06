#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControl.h"

//Core.
#include <Core/Essential/CatalystEssential.h>

enum EParams
{
  BOOST,
  CEILING,
  NUMBER_OF_PARAMS
};

class DaybreakLimiter : public iplug::Plugin
{

public:

  DaybreakLimiter(const iplug::InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
#endif

private:

  //The text. (:
  iplug::igraphics::ITextControl *RESTRICT _Text{ nullptr };

  //The boost.
  float32 _Boost{ 1.0f };

  //The ceiling.
  float32 _Ceiling{ 1.0f };

};
