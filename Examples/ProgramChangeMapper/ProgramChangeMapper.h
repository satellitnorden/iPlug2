#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>
#include <Core/Containers/StaticArray.h>
#include <Core/General/DynamicString.h>
#include <Core/General/HashString.h>

//IPlug2
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

//Program Change Mapper save data class definition.
class ProgramChangeMapperSaveData final
{

public:

  //The current save version.
  constexpr static uint64 CURRENT_SAVE_VERSION{ 1 };

  //The version.
  uint32 _Version;

  //The mapping name.
  char _MappingName[128];

  /*
  * Resets the save data.
  */
  void Reset()
  {
    _Version = CURRENT_SAVE_VERSION;
    sprintf_s(_MappingName, "None");
  }

};

enum class InternalParameter : uint8
{
  MAPPING
};

enum EParams
{
  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class ProgramChangeMapper final : public Plugin
{

public:

  friend class DropDownMenu;

  ProgramChangeMapper(const InstanceInfo& info);
  ~ProgramChangeMapper();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnReset() override;
  void ProcessBlock(sample** inputs, sample** outputs, int number_of_frames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
#endif

  bool SerializeState(iplug::IByteChunk &chunk) const override;
  int UnserializeState(const iplug::IByteChunk& chunk, int start_position) override;

  void SetInternalParameterIndex(const InternalParameter internal_parameter, const uint64 index);

private:

  /*
  * Mapping component class definition.
  */
  class MappingComponent final
  {


  public:

    //The source.
    int32 _Source;

    //The destination.
    int32 _Destination;

  };

  /*
  * Mapping class definition.
  */
  class Mapping final
  {


  public:

    //The name.
    DynamicString _Name;

    //The components.
    DynamicArray<MappingComponent> _Components;

  };

  //The mappings.
  DynamicArray<Mapping> _Mappings;

  //The current mapping index.
  uint64 _CurrentMappingIndex{ UINT64_MAXIMUM };

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The mappings menu.
  IPopupMenu _MappingsMenu;

  //The save data.
  ProgramChangeMapperSaveData _SaveData;

  // Denotes if save data has been received.
  bool _SaveDataHasBeenReceived{ false };

  //Denotes if data has been read.
  bool _DataHasBeenRead{ false };

  //Denotes if data has been synchronized.
  bool _DataHasBeenSynchronised{ false };

  /*
  * Reads the mappings.
  */
  void ReadMappings() NOEXCEPT;

};
