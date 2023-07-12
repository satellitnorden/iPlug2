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

//Drum Mapper save data class definition.
class DrumMapperSaveData final
{

public:

  //The current save version.
  constexpr static uint64 CURRENT_SAVE_VERSION{ 1 };

  //The version.
  uint32 _Version;

  //The input mapping name.
  char _InputMappingName[128];

  //The mapping protocol 1 name.
  char _MappingProtocol1Name[128];

  //The mapping protocol 2 name.
  char _MappingProtocol2Name[128];

  //The output mapping name.
  char _OutputMappingName[128];

  /*
  * Resets the save data.
  */
  void Reset()
  {
    _Version = CURRENT_SAVE_VERSION;
    sprintf_s(_InputMappingName, "None");
    sprintf_s(_MappingProtocol1Name, "None");
    sprintf_s(_MappingProtocol2Name, "None");
    sprintf_s(_OutputMappingName, "None");
  }

};

enum class InternalParameter : uint8
{
  INPUT_MAPPING,
  MAPPING_PROTOCOL_1,
  MAPPING_PROTOCOL_2,
  OUTPUT_MAPPING
};

enum EParams
{
  NUMBER_OF_PARAMS
};

using namespace iplug;
using namespace igraphics;

class DrumMapper final : public Plugin
{

public:

  friend class DropDownMenu;

  DrumMapper(const InstanceInfo& info);
  ~DrumMapper();

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

    //The name.
    DynamicString _Name;

    //The components.
    DynamicArray<MappingComponent> _Components;

    /*
    * Returns the index of the given identifier in this mapping.
    * Returns UINT64_MAXIMUM if none was found.
    */
    uint64 FindIdentifier(const HashString identifier)
    {
      for (uint64 i{ 0 }; i < _Components.Size(); ++i)
      {
        if (_Components[i]._Identifier == identifier)
        {
          return i;
        }
      }

      return UINT64_MAXIMUM;
    }

  };

  /*
  * Mapping protocol component class definition.
  */
  class MappingProtocolComponent final
  {


  public:

    //The first identifier.
    HashString _FirstIdentifier;

    //The second identifier.
    HashString _SecondIdentifier;

  };

  /*
  * Mapping protocol class definition.
  */
  class MappingProtocol final
  {


  public:

    //The name.
    DynamicString _Name;

    //The components.
    DynamicArray<MappingProtocolComponent> _Components;

  };

  //The mappings.
  DynamicArray<Mapping> _Mappings;

  //The mapping protocols.
  DynamicArray<MappingProtocol> _MappingProtocols;

  //The current input mapping index.
  uint64 _CurrentInputMappingIndex{ UINT64_MAXIMUM };

  //The current mapping protocol indices.
  StaticArray<uint64, 2> _CurrentMappingProtocolIndices;

  //The current output mapping index.
  uint64 _CurrentOutputMappingIndex{ UINT64_MAXIMUM };

  //The midi queue.
  iplug::IMidiQueue _MidiQueue;

  //The mappings menu.
  IPopupMenu _MappingsMenu;

  //The mapping protocols menu.
  IPopupMenu _MappingProtocolsMenu;

  //The samples since last trigger.
  DynamicArray<uint64> _SamplesSinceLastTrigger;

  //The save data.
  DrumMapperSaveData _SaveData;

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

  /*
  * Reads the mapping protocols.
  */
  void ReadMappingProtocols() NOEXCEPT;

};
