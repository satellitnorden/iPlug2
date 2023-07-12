//Header file.
#include "ProgramChangeMapper.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//File.
#include <File/Core/FileCore.h>

//STL.
#include <filesystem>

//Windows.
#include <ShlObj_core.h>

/*
* Dropdown menu class definition.
*/
class DropDownMenu final : public IControl
{

public:

  DropDownMenu(ProgramChangeMapper* const RESTRICT plugin, const iplug::igraphics::IRECT& bounds, const InternalParameter internal_parameter)
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

  ProgramChangeMapper *RESTRICT _Plugin;
  InternalParameter _InternalParameter;
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };

  void CalculateName()
  {
    switch (_InternalParameter)
    {
      case InternalParameter::MAPPING:
      {
        const uint64 index{ _Plugin->_CurrentMappingIndex };

        if (index == UINT64_MAXIMUM)
        {
          _Name = "Mapping: None";
        }

        else
        {
          char buffer[128];
          sprintf_s(buffer, "Mapping: %s", _Plugin->_Mappings[index]._Name.Data());

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
    IPopupMenu &popup_menu{ _Plugin->_MappingsMenu };

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

ProgramChangeMapper::ProgramChangeMapper(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Read the mappings.
  ReadMappings();

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
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", PROGRAMCHANGEMAPPER_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the panel background.
    pGraphics->AttachBackground(PROGRAMCHANGEMAPPER_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(-96).GetMidVPadded(24), InternalParameter::MAPPING));
  };
#endif
}

ProgramChangeMapper::~ProgramChangeMapper()
{

}

#if IPLUG_DSP
void ProgramChangeMapper::OnReset()
{
  
}

void ProgramChangeMapper::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  if (!_DataHasBeenSynchronised
      && _SaveDataHasBeenReceived
      && _DataHasBeenRead)
  {
    for (uint64 i{ 0 }; i < _Mappings.Size(); ++i)
    {
      if (_Mappings[i]._Name == _SaveData._MappingName)
      {
        _CurrentMappingIndex = i;

        break;
      }
    }

    _DataHasBeenSynchronised = true;
  }

  for (int32 i{ 0 }; i < number_of_frames; ++i)
  {
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > i)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::kControlChange)
      {
        //If mapping isn't defined, just let the message pass through.
        if (_CurrentMappingIndex == UINT64_MAXIMUM)
        {
          SendMidiMsg(message);
        }

        else
        {
          const int32 input_program{ message.mData1 };

          for (const MappingComponent& mapping_component : _Mappings[_CurrentMappingIndex]._Components)
          {
            if (input_program == mapping_component._Source)
            {
              iplug::IMidiMsg output_message{ message };

              output_message.mData1 = mapping_component._Destination;

              SendMidiMsg(output_message);

              break;
            }
          }
        }
      }

      _MidiQueue.Remove();
    }
  }
}

void ProgramChangeMapper::ProcessMidiMsg(const IMidiMsg& msg)
{
   _MidiQueue.Add(msg);
}

bool ProgramChangeMapper::SerializeState(iplug::IByteChunk &chunk) const
{
  const bool parent_succeeded{ iplug::Plugin::SerializeState(chunk) };

  chunk.PutBytes(&_SaveData, sizeof(ProgramChangeMapperSaveData));

  return parent_succeeded;

}

int ProgramChangeMapper::UnserializeState(const iplug::IByteChunk& chunk, int start_position)
{
  const int32 parent_position{ iplug::Plugin::UnserializeState(chunk, start_position) };

  chunk.GetBytes(&_SaveData, sizeof(ProgramChangeMapperSaveData), parent_position);

  if (_SaveData._Version == 0)
  {
    _SaveData.Reset();
  }

  _SaveDataHasBeenReceived = true;

  return parent_position + sizeof(ProgramChangeMapperSaveData);
}

void ProgramChangeMapper::SetInternalParameterIndex(const InternalParameter internal_parameter, const uint64 index)
{
  switch (internal_parameter)
  {
    case InternalParameter::MAPPING:
    {
      _CurrentMappingIndex = index - 1;

      if (_CurrentMappingIndex != UINT64_MAXIMUM)
      {
        sprintf_s(_SaveData._MappingName, _Mappings[_CurrentMappingIndex]._Name.Data());
      }

      else
      {
        sprintf_s(_SaveData._MappingName, "None");
      }

      break;
    }
  }
}

/*
* Reads the mappings.
*/
void ProgramChangeMapper::ReadMappings() NOEXCEPT
{
  //Clear the mappings.
  _Mappings.Clear();

  /*
  //Retrieve the data path.
#if APP_API
  DynamicString data_path{ RetrievePluginPath("ProgramChangeMapper.exe") };
#elif VST3_API
  DynamicString data_path{RetrievePluginPath("ProgramChangeMapper.vst3")};
#endif

  data_path += "ProgramChangeMapperData";
  */

  DynamicString data_path = "C:\\Users\\Dennis\\My Drive\\Share Folder\\Plugins\\ProgramChangeMapperData";

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

      //Set the source.
      new_mapping_component._Source = std::stoi(identifier.c_str());

      //Set the destination.
      new_mapping_component._Destination = std::stoi(argument.c_str());
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
#endif