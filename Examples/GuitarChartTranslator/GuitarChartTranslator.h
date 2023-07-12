#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/StaticArray.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

enum EParams
{
  STRING_1_TUNING,
  STRING_2_TUNING,
  STRING_3_TUNING,
  STRING_4_TUNING,
  STRING_5_TUNING,
  STRING_6_TUNING,
  STRING_7_TUNING,
  STRING_8_TUNING,
  STRING_9_TUNING,
  STRING_10_TUNING,

  DESTINATION_LIBRARY,

  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class GuitarChartTranslator final : public Plugin
{

public:

  //Enumeration covering all destination libraries.
  enum class DestinationLibrary : uint8
  {
    NONE,

    SHREDDAGE_3_HYDRA,

    NUMBER_OF_DESTINATION_LIBRARIES
  };

  friend class DropDownMenu;

  GuitarChartTranslator(const InstanceInfo& info);
  ~GuitarChartTranslator();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnParamChange(int index) override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

private:

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The current destination library.
  DestinationLibrary _CurrentDestinationLibrary{ DestinationLibrary::NONE };

  //The destination library menu.
  IPopupMenu _DestinationLibraryMenu;

  //The string tunings.
  StaticArray<int32, 10> _StringTunings;

  /*
  * Translates.
  */
  void Translate(const iplug::IMidiMsg &message) NOEXCEPT;

};
