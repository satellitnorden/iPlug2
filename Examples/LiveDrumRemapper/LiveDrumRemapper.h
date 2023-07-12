#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>
#include <Core/General/DynamicString.h>
#include <Core/General/HashString.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

enum EParams
{
  INPUT_MAPPING,
  MAPPING_PROTOCOL,
  OUTPUT_MAPPING,

  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class LiveDrumRemapper final : public Plugin
{

public:

  friend class DropDownMenu;

  LiveDrumRemapper(const InstanceInfo& info);
  ~LiveDrumRemapper();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void OnParamChange(int index) override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

private:

  /*
  * Mapping component class definition.
  */
  class MappingComponent final
  {


  public:

   //The identifier.
    HashString _Identifier;

    //The MIDI note.
    int32 _MIDINote;

  };

  /*
  * Mapping class definition.
  */
  class Mapping final
  {


  public:

    //The mapping name.
    DynamicString _MappingName;

    //The components.
    DynamicArray<MappingComponent> _Components;

  };

  //The mappings.
  DynamicArray<Mapping> _Mappings;

  //The current input mapping index.
  int _CurrentInputMappingIndex{ 0 };

  //The current output mapping index.
  int _CurrentOutputMappingIndex{ 0 };

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The mappings menu.
  IPopupMenu _MappingsMenu;

  /*
  * Reads the mappings.
  */
  void ReadMappings() NOEXCEPT;

};
