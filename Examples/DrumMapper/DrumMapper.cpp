//Header file.
#include "DrumMapper.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//File.
#include <File/Core/FileCore.h>

//STL.
#include <filesystem>

//Windows.
#include <ShlObj_core.h>

//Drum Mapper constants.
namespace DrumMapperConstants
{
  constexpr uint32 MINIMUM_SAMPLES_BETWEEN_TRIGGERS{ 1'024 };
}

uint64 RoundToIndex(const double value)
{
  return static_cast<uint64>(value + 0.5);
}

/*
* Dropdown menu class definition.
*/
class DropDownMenu final : public IControl
{

public:

  DropDownMenu(DrumMapper* const RESTRICT plugin, const iplug::igraphics::IRECT& bounds, const InternalParameter internal_parameter)
    :
    IControl(bounds)
  {
    _Plugin = plugin;
    _InternalParameter = internal_parameter;
  }

  void OnMouseDown(float X, float Y, const IMouseMod &mod) override
  {
    IControl::OnMouseDown(X, Y, mod);

    _CurrentHoverState = HoverState::CLICKED;

    Execute();

    SetDirty();
  }

  void OnMouseOver(float X, float Y, const IMouseMod &mod) override
  {
    IControl::OnMouseOver(X, Y, mod);

    if (_CurrentHoverState != HoverState::CLICKED)
    {
      _CurrentHoverState = HoverState::HOVERED;
    }

    SetDirty();
  }

  void OnMouseOut() override
  {
    IControl::OnMouseOut();

    if (_CurrentHoverState != HoverState::CLICKED)
    {
      _CurrentHoverState = HoverState::IDLE;
    }

    SetDirty();
  }

  void Draw(IGraphics &graphics) override
  {
    CalculateName();

    graphics.FillRect(GetButtonColor(), mRECT);
    graphics.DrawText(IText(16, IColor(255, 255, 255, 255)), _Name.Data(), mRECT);
  }

private:

  enum class HoverState
  {
    IDLE,
    HOVERED,
    CLICKED
  };

  DrumMapper *RESTRICT _Plugin;
  InternalParameter _InternalParameter;
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };

  void CalculateName()
  {
    switch (_InternalParameter)
    {
      case InternalParameter::INPUT_MAPPING:
      {
        const uint64 index{ _Plugin->_CurrentInputMappingIndex };

        if (index == UINT64_MAXIMUM)
        {
          _Name = "Input Mapping: None";
        }

        else
        {
          char buffer[128];
          sprintf_s(buffer, "Input Mapping: %s", _Plugin->_Mappings[index]._Name.Data());

          _Name = buffer;
        }

        break;
      }

      case InternalParameter::MAPPING_PROTOCOL_1:
      {
        const uint64 index{ _Plugin->_CurrentMappingProtocolIndices[0] };

        if (index == UINT64_MAXIMUM)
        {
          _Name = "Mapping Protocol 1: None";
        }

        else
        {
          char buffer[128];
          sprintf_s(buffer, "Mapping Protocol 1: %s", _Plugin->_MappingProtocols[index]._Name.Data());

          _Name = buffer;
        }

        break;
      }

      case InternalParameter::MAPPING_PROTOCOL_2:
      {
        const uint64 index{ _Plugin->_CurrentMappingProtocolIndices[1] };

        if (index == UINT64_MAXIMUM)
        {
          _Name = "Mapping Protocol 2: None";
        }

        else
        {
          char buffer[128];
          sprintf_s(buffer, "Mapping Protocol 2: %s", _Plugin->_MappingProtocols[index]._Name.Data());

          _Name = buffer;
        }

        break;
      }

      case InternalParameter::OUTPUT_MAPPING:
      {
        const uint64 index{ _Plugin->_CurrentOutputMappingIndex };

        if (index == UINT64_MAXIMUM)
        {
          _Name = "Output Mapping: None";
        }

        else
        {
          char buffer[128];
          sprintf_s(buffer, "Output Mapping: %s", _Plugin->_Mappings[index]._Name.Data());

          _Name = buffer;
        }

        break;
      }

      default:
      {
        _Name = "ERROR";

        break;
      }
    }
  }

  IColor GetButtonColor()
  {
    switch (_CurrentHoverState)
    {
      case HoverState::IDLE:
      {
        return IColor(200, 25, 25, 25);
      }

      case HoverState::HOVERED:
      {
        return IColor(200, 75, 75, 75);
      }

      case HoverState::CLICKED:
      {
        return IColor(200, 125, 125, 125);
      }

      default:
      {
        return IColor(200, 25, 25, 25);
      }
    }
  }

  void Execute()
  {
    IPopupMenu &popup_menu{ (_InternalParameter == InternalParameter::MAPPING_PROTOCOL_1 || _InternalParameter == InternalParameter::MAPPING_PROTOCOL_2) ? _Plugin->_MappingProtocolsMenu : _Plugin->_MappingsMenu };

    _Plugin->GetUI()->CreatePopupMenu(*this, popup_menu, mRECT);

    const int chosen_item_index{ popup_menu.GetChosenItemIdx() };

    if (chosen_item_index > -1)
    {
      _Plugin->SetInternalParameterIndex(_InternalParameter, chosen_item_index);
    }

    if (mMouseIsOver)
    {
      _CurrentHoverState = HoverState::HOVERED;
    }

    else
    {
      _CurrentHoverState = HoverState::IDLE;
    }
  }

};

/*
* Dummy function.
*/
void DummyFunction()
{

}

/*
 * Retrieves the path of this plugin.
 */
DynamicString RetrievePluginPath(const char *const RESTRICT plugin_name) NOEXCEPT
{
  //Retrieve the folder.
  char file_path[MAX_PATH];
  HMODULE module_handle{ nullptr };

  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&DummyFunction, &module_handle) == 0)
  {
    return DynamicString();
  }

  if (GetModuleFileNameA(module_handle, file_path, sizeof(file_path)) == 0)
  {
    return DynamicString();
  }

  //Skip the actual plugin name.
  std::string file_path_string{file_path};

  {
    std::string from{ std::string("\\") + std::string(plugin_name) };
    size_t start_position{ file_path_string.find(from) };

    if (start_position != std::string::npos)
    {
      file_path_string.replace(start_position, from.length(), "\\");
    }
  }

  return DynamicString(file_path_string.c_str());
}

DrumMapper::DrumMapper(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Reset the current mapping protocol indices.
  for (uint64 i{ 0 }; i < _CurrentMappingProtocolIndices.Size(); ++i)
  {
    _CurrentMappingProtocolIndices[i] = UINT64_MAXIMUM;
  }

  //Read the mappings.
  ReadMappings();

  //Read the mapping protocols.
  ReadMappingProtocols();

  _DataHasBeenRead = true;

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Enable mouse over events.
    pGraphics->EnableMouseOver(true);

    //Load the font.
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", DRUMMAPPER_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the panel background.
    pGraphics->AttachBackground(DRUMMAPPER_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(-96).GetMidVPadded(24), InternalParameter::INPUT_MAPPING));
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(-32).GetMidVPadded(24), InternalParameter::MAPPING_PROTOCOL_1));
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(32).GetMidVPadded(24), InternalParameter::MAPPING_PROTOCOL_2));
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(96).GetMidVPadded(24), InternalParameter::OUTPUT_MAPPING));
  };
#endif
}

DrumMapper::~DrumMapper()
{

}

#if IPLUG_DSP
void DrumMapper::OnReset()
{
  
}

void DrumMapper::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  if (!_DataHasBeenSynchronised
      && _SaveDataHasBeenReceived
      && _DataHasBeenRead)
  {
    for (uint64 i{ 0 }; i < _Mappings.Size(); ++i)
    {
      if (_Mappings[i]._Name == _SaveData._InputMappingName)
      {
        _CurrentInputMappingIndex = i;
      }

      if (_Mappings[i]._Name == _SaveData._OutputMappingName)
      {
        _CurrentOutputMappingIndex = i;

        _SamplesSinceLastTrigger.Clear();

        _SamplesSinceLastTrigger.Upsize<false>(_Mappings[_CurrentOutputMappingIndex]._Components.Size());

        for (uint64 &value : _SamplesSinceLastTrigger)
        {
          value = DrumMapperConstants::MINIMUM_SAMPLES_BETWEEN_TRIGGERS;
        }
      }
    }

    for (uint64 i{ 0 }; i < _MappingProtocols.Size(); ++i)
    {
      if (_MappingProtocols[i]._Name == _SaveData._MappingProtocol1Name)
      {
        _CurrentMappingProtocolIndices[0] = i;
      }

      if (_MappingProtocols[i]._Name == _SaveData._MappingProtocol2Name)
      {
        _CurrentMappingProtocolIndices[1] = i;
      }
    }

    _DataHasBeenSynchronised = true;
  }

  for (int32 i{ 0 }; i < number_of_frames; ++i)
  {
    for (uint64 &value : _SamplesSinceLastTrigger)
    {
      ++value;
    }

    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > i)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::kNoteOn
        || status == iplug::IMidiMsg::kNoteOff
        || status == iplug::IMidiMsg::kPolyAftertouch)
      {
        const int32 input_note_number{ message.NoteNumber() };

        //If input and output mappings aren't defined, just let the message pass through.
        if (_CurrentInputMappingIndex == UINT64_MAXIMUM
            || _CurrentOutputMappingIndex == UINT64_MAXIMUM)
        {
          SendMidiMsg(message);
        }

        else
        {
          for (const MappingComponent& input_mapping_component : _Mappings[_CurrentInputMappingIndex]._Components)
          {
            if (input_note_number == input_mapping_component._MIDINote)
            {
              uint64 output_mapping_component_index{ _Mappings[_CurrentOutputMappingIndex].FindIdentifier(input_mapping_component._Identifier) };

              if (output_mapping_component_index != UINT64_MAXIMUM)
              {
                if (status == iplug::IMidiMsg::kNoteOn
                    && _SamplesSinceLastTrigger[output_mapping_component_index] < DrumMapperConstants::MINIMUM_SAMPLES_BETWEEN_TRIGGERS)
                {
                  output_mapping_component_index = UINT64_MAXIMUM;
                }
              }

              if (output_mapping_component_index == UINT64_MAXIMUM)
              {
                for (uint64 i{ 0 }; i < _CurrentMappingProtocolIndices.Size(); ++i)
                {
                  if (_CurrentMappingProtocolIndices[i] == UINT64_MAXIMUM)
                  {
                    continue;
                  }

                  if (output_mapping_component_index != UINT64_MAXIMUM)
                  {
                    break;
                  }

                  for (const MappingProtocolComponent& mapping_protocol_component : _MappingProtocols[_CurrentMappingProtocolIndices[i]]._Components)
                  {
                    if (mapping_protocol_component._FirstIdentifier == input_mapping_component._Identifier)
                    {
                      output_mapping_component_index = _Mappings[_CurrentOutputMappingIndex].FindIdentifier(mapping_protocol_component._SecondIdentifier);

                      if (output_mapping_component_index != UINT64_MAXIMUM)
                      {
                        if (status == iplug::IMidiMsg::kNoteOn
                            && _SamplesSinceLastTrigger[output_mapping_component_index] < DrumMapperConstants::MINIMUM_SAMPLES_BETWEEN_TRIGGERS)
                        {
                          output_mapping_component_index = UINT64_MAXIMUM;
                        }

                        else
                        {
                          break;
                        }
                      }
                    }
                  }
                }
              }

              if (output_mapping_component_index != UINT64_MAXIMUM)
              {
                iplug::IMidiMsg output_message{ message };

                output_message.mData1 = _Mappings[_CurrentOutputMappingIndex]._Components[output_mapping_component_index]._MIDINote;

                SendMidiMsg(output_message);

                if (status == iplug::IMidiMsg::kNoteOn)
                {
                  _SamplesSinceLastTrigger[output_mapping_component_index] = 0;
                }
              }

              break;
            }
          }
        }
      }

      _MidiQueue.Remove();
    }
  }
}

void DrumMapper::ProcessMidiMsg(const IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}

bool DrumMapper::SerializeState(iplug::IByteChunk &chunk) const
{
  const bool parent_succeeded{ iplug::Plugin::SerializeState(chunk) };

  chunk.PutBytes(&_SaveData, sizeof(DrumMapperSaveData));

  return parent_succeeded;

}

int DrumMapper::UnserializeState(const iplug::IByteChunk& chunk, int start_position)
{
  const int32 parent_position{ iplug::Plugin::UnserializeState(chunk, start_position) };

  chunk.GetBytes(&_SaveData, sizeof(DrumMapperSaveData), parent_position);

  if (_SaveData._Version == 0)
  {
    _SaveData.Reset();
  }

  _SaveDataHasBeenReceived = true;

  return parent_position + sizeof(DrumMapperSaveData);
}

void DrumMapper::SetInternalParameterIndex(const InternalParameter internal_parameter, const uint64 index)
{
  switch (internal_parameter)
  {
    case InternalParameter::INPUT_MAPPING:
    {
      _CurrentInputMappingIndex = index - 1;

      if (_CurrentInputMappingIndex != UINT64_MAXIMUM)
      {
        sprintf_s(_SaveData._InputMappingName, _Mappings[_CurrentInputMappingIndex]._Name.Data());
      }

      else
      {
        sprintf_s(_SaveData._InputMappingName, "None");
      }

      break;
    }

    case InternalParameter::MAPPING_PROTOCOL_1:
    {
      _CurrentMappingProtocolIndices[0] = index - 1;

      if (_CurrentMappingProtocolIndices[0] != UINT64_MAXIMUM)
      {
        sprintf_s(_SaveData._MappingProtocol1Name, _MappingProtocols[_CurrentMappingProtocolIndices[0]]._Name.Data());
      }

      else
      {
        sprintf_s(_SaveData._MappingProtocol1Name, "None");
      }

      break;
    }

    case InternalParameter::MAPPING_PROTOCOL_2:
    {
      _CurrentMappingProtocolIndices[1] = index - 1;

      if (_CurrentMappingProtocolIndices[1] != UINT64_MAXIMUM)
      {
        sprintf_s(_SaveData._MappingProtocol2Name, _MappingProtocols[_CurrentMappingProtocolIndices[1]]._Name.Data());
      }

      else
      {
        sprintf_s(_SaveData._MappingProtocol2Name, "None");
      }

      break;
    }

    case InternalParameter::OUTPUT_MAPPING:
    {
      const uint64 previous_output_mapping_index{ _CurrentOutputMappingIndex };

      _CurrentOutputMappingIndex = index - 1;

      if (_CurrentOutputMappingIndex != UINT64_MAXIMUM)
      {
        sprintf_s(_SaveData._OutputMappingName, _Mappings[_CurrentOutputMappingIndex]._Name.Data());
      }

      else
      {
        sprintf_s(_SaveData._OutputMappingName, "None");
      }

      if (previous_output_mapping_index != _CurrentOutputMappingIndex && _CurrentOutputMappingIndex != UINT64_MAXIMUM)
      {
        _SamplesSinceLastTrigger.Clear();

        _SamplesSinceLastTrigger.Upsize<false>(_Mappings[_CurrentOutputMappingIndex]._Components.Size());

        for (uint64& value : _SamplesSinceLastTrigger)
        {
          value = DrumMapperConstants::MINIMUM_SAMPLES_BETWEEN_TRIGGERS;
        }
      }

      break;
    }
  }
}

/*
* Reads the mappings.
*/
void DrumMapper::ReadMappings() NOEXCEPT
{
  //Clear the mappings.
  _Mappings.Clear();

  //Retrieve the data path.
#if APP_API
  DynamicString data_path{ RetrievePluginPath("DrumMapper.exe") };
#elif VST3_API
  DynamicString data_path{RetrievePluginPath("DrumMapper.vst3")};
#endif

  data_path += "DrumMapperData\\Mappings";

  //Iterate over all files in this directory.
  for (const auto& entry : std::filesystem::directory_iterator(std::string(data_path.Data())))
  {
    //Skip directories, for now.
    if (entry.is_directory())
    {
      continue;
    }

    //Retrieve the file path.
    const std::string file_path{ entry.path().string() };

    //Retrieve the extension.
    const File::Extension extension{ File::GetExtension(file_path.c_str()) };

    //Start constructing the new mapping.
    _Mappings.Emplace();
    Mapping &new_mapping{ _Mappings.Back() };

    {
      const size_t start{ file_path.find_last_of("\\") + 1 };
      const size_t end{ file_path.find_last_of(".txt") };

      new_mapping._Name = file_path.substr(start, end - start - 3).c_str();
    }

    //Process the file.
    std::ifstream file{ file_path };

    std::string current_line;
    std::string identifier;
    std::string argument;

    // Go over all the lines.
    while (std::getline(file, current_line))
    {
      //Ignore comments.
      if (current_line[0] == '#')
      {
        continue;
      }

      //Ignore empty lines.
      if (current_line.empty())
      {
        continue;
      }

      // Split the current line into identifier and argument.
      const size_t space_position{current_line.find_first_of(" ")};

      if (space_position != std::string::npos)
      {
        identifier = current_line.substr(0, space_position);
        argument = current_line.substr(space_position + 1, std::string::npos);
      }

      else
      {
        continue;
      }

      //Add the new mapping component.
      new_mapping._Components.Emplace();
      MappingComponent &new_mapping_component{ new_mapping._Components.Back() };

      //Calculate the identifier.
      new_mapping_component._Identifier = HashString(identifier.c_str());

      //Calculate the MIDI note.
      new_mapping_component._MIDINote = std::stoi(argument.c_str());
    }

    //Close the file.
    file.close();
  }

  //Construct the mappings menu.
  _MappingsMenu.Clear();

  _MappingsMenu.AddItem("None");

  for (const Mapping& mapping : _Mappings)
  {
    _MappingsMenu.AddItem(mapping._Name.Data());
  }
}

/*
 * Reads the mapping protocols.
 */
void DrumMapper::ReadMappingProtocols() NOEXCEPT
{
  //Clear the mapping protocols.
  _MappingProtocols.Clear();

  //Retrieve the mappings path.
#if APP_API
  DynamicString data_path{RetrievePluginPath("DrumMapper.exe")};
#elif VST3_API
  DynamicString data_path{RetrievePluginPath("DrumMapper.vst3")};
#endif

  data_path += "DrumMapperData\\MappingProtocols";

  // Iterate over all files in this directory.
  for (const auto& entry : std::filesystem::directory_iterator(std::string(data_path.Data())))
  {
    // Skip directories, for now.
    if (entry.is_directory())
    {
      continue;
    }

    // Retrieve the file path.
    const std::string file_path{ entry.path().string() };

    // Retrieve the extension.
    const File::Extension extension{ File::GetExtension(file_path.c_str()) };

    //Start constructing the new mapping protocol.
    _MappingProtocols.Emplace();
    MappingProtocol &new_mapping_protocol{ _MappingProtocols.Back() };

    {
      const size_t start{ file_path.find_last_of("\\") + 1 };
      const size_t end{ file_path.find_last_of(".txt") };

      new_mapping_protocol._Name = file_path.substr(start, end - start - 3).c_str();
    }

    //Process the file.
    std::ifstream file{file_path};

    std::string current_line;
    std::string identifier;
    std::string argument;

    //Go over all the lines.
    while (std::getline(file, current_line))
    {
      //Ignore comments.
      if (current_line[0] == '#')
      {
        continue;
      }

      //Ignore empty lines.
      if (current_line.empty())
      {
        continue;
      }

      //Split the current line into identifier and argument.
      const size_t space_position{current_line.find_first_of(" ")};

      if (space_position != std::string::npos)
      {
        identifier = current_line.substr(0, space_position);
        argument = current_line.substr(space_position + 1, std::string::npos);
      }

      else
      {
        continue;
      }

      //Add the new mapping protocol component.
      new_mapping_protocol._Components.Emplace();
      MappingProtocolComponent &new_mapping_component{ new_mapping_protocol._Components.Back() };

      //Calculate the first identifier.
      new_mapping_component._FirstIdentifier = HashString(identifier.c_str());

      //Calculate the second identifier.
      new_mapping_component._SecondIdentifier = HashString(argument.c_str());
    }

    //Close the file.
    file.close();
  }

  //Construct the mapping protocols menu.
  _MappingProtocolsMenu.Clear();

  _MappingProtocolsMenu.AddItem("None");

  for (const MappingProtocol &mapping_protocol : _MappingProtocols)
  {
    _MappingProtocolsMenu.AddItem(mapping_protocol._Name.Data());
  }
}
#endif