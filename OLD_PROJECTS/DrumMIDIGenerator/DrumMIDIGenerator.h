#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControl.h"

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/StaticArray.h>

class DrumMIDIGenerator : public iplug::Plugin
{

public:

  DrumMIDIGenerator(const iplug::InstanceInfo& info);
  ~DrumMIDIGenerator() override;

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnActivate(bool value) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The output log.
  std::ofstream _OutputLog;

  iplug::igraphics::ITextControl* _Text;

};
