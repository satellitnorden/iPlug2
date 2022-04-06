#pragma once

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Core.
#include <Core/Essential/CatalystEssential.h>

//Procedural Musician.
#include "OutgoingMIDIEvent.h"
#include "SectionGenerator.h"

#define USE_OUTPUT_LOG (0)

enum EParams
{
  NUMBER_OF_PARAMS
};

class ProceduralMusician final : public iplug::Plugin
{
public:

  ProceduralMusician(const iplug::InstanceInfo& info);
  ~ProceduralMusician();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(const int32 index) override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif

private:

  //The incoming midi queue.
  iplug::IMidiQueue _IncomingMidiQueue;

  //The outgoing midi queue.
  DynamicArray<OutgoingMIDIEvent> _OutgoingMidiQueue;

  //The section generators.
  DynamicArray<SectionGenerator *RESTRICT> _SectionGenerators;

  //Denotes the current section generator.
  uint64 _CurrentSectionGenerator{ 0 };

#if USE_OUTPUT_LOG
  //The output log.
  std::ofstream _OutputLog;
#endif

  /*
  * Returns the current section generator.
  */
  FORCE_INLINE RESTRICTED SectionGenerator *const RESTRICT GetCurrentSectionGenerator() NOEXCEPT
  {
    return _SectionGenerators[_CurrentSectionGenerator];
  }

};
