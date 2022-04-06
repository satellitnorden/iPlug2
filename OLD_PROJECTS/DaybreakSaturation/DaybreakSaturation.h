#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControl.h"

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

enum EParams
{
  VARIATION,
  BOOST,
  NUMBER_OF_PARAMS
};

class DaybreakSaturation : public iplug::Plugin
{

public:

  DaybreakSaturation(const iplug::InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The text. (:
  iplug::igraphics::ITextControl *RESTRICT _Text{ nullptr };

  //The variation.
  uint8 _Variation{ 0 };

  //The boost.
  float32 _Boost{ 1.0f };

};
