//Header file.
#include "LiveDrumRemapper.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//File.
#include <File/Core/FileCore.h>

//STL.
#include <filesystem>

//Windows.
#include <ShlObj_core.h>

	#define DEBUG_OUTPUT(MESSAGE)                                                                                                                                                                       \
 {                                                                                                                                                                                                    \
    std::ostringstream output;                                                                                                                                                                         \
    output << MESSAGE << std::endl;                                                                                                                                                                    \
    OutputDebugString(output.str().c_str());                                                                                                                                             \
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

  DropDownMenu(LiveDrumRemapper* const RESTRICT plugin, const iplug::igraphics::IRECT& bounds, const int parameter_index)
    :
    IControl(bounds, parameter_index)
  {
    _Plugin = plugin;
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

  LiveDrumRemapper *RESTRICT _Plugin;
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };

  void CalculateName()
  {
    switch (GetParamIdx())
    {
      case 0:
      {
        char buffer[128];
        sprintf_s(buffer, "Input Mapping: %s", _Plugin->_Mappings[RoundToIndex(_Plugin->GetParam(INPUT_MAPPING)->Value())]._MappingName.Data());

        _Name = buffer;

        break;
      }

      case 1:
      {
        char buffer[128];
        sprintf_s(buffer, "Mapping Protocol: %s", _Plugin->_Mappings[RoundToIndex(_Plugin->GetParam(MAPPING_PROTOCOL)->Value())]._MappingName.Data());

        _Name = buffer;

        break;
      }

      case 2:
      {
        char buffer[128];
        sprintf_s(buffer, "Output Mapping: %s", _Plugin->_Mappings[RoundToIndex(_Plugin->GetParam(OUTPUT_MAPPING)->Value())]._MappingName.Data());

        _Name = buffer;

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
      SetValue(static_cast<double>(chosen_item_index) / static_cast<double>(popup_menu.NItems()));
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

LiveDrumRemapper::LiveDrumRemapper(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Read the mappings.
  ReadMappings();

  //Set up the parameters.
  GetParam(INPUT_MAPPING)->InitInt("INPUT_MAPPING", 0, 0, static_cast<int32>(_Mappings.Size()));
  GetParam(MAPPING_PROTOCOL)->InitInt("MAPPING_PROTOCOL", 0, 0, static_cast<int32>(_Mappings.Size()));
  GetParam(OUTPUT_MAPPING)->InitInt("OUTPUT_MAPPING", 0, 0, static_cast<int32>(_Mappings.Size()));
    
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
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", LIVEDRUMREMAPPER_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the panel background.
    pGraphics->AttachBackground(LIVEDRUMREMAPPER_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(256).GetVShifted(-64).GetMidVPadded(24), INPUT_MAPPING));
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(256).GetVShifted(0).GetMidVPadded(24), MAPPING_PROTOCOL));
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(256).GetVShifted(64).GetMidVPadded(24), OUTPUT_MAPPING));
  };
#endif
}

LiveDrumRemapper::~LiveDrumRemapper()
{

}

#if IPLUG_DSP
void LiveDrumRemapper::OnReset()
{
  
}

void LiveDrumRemapper::OnParamChange(int index)
{
  switch (index)
  {
    case INPUT_MAPPING:
    {
      _CurrentInputMappingIndex = RoundToIndex(GetParam(index)->Value());

      break;
    }

    case OUTPUT_MAPPING:
    {
      _CurrentOutputMappingIndex = RoundToIndex(GetParam(index)->Value());

      break;
    }
  }
}
  void LiveDrumRemapper::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
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

      if (status == iplug::IMidiMsg::kNoteOn
          || status == iplug::IMidiMsg::kNoteOff)
      {
        const int32 input_note_number{ message.mData1 };

        for (const MappingComponent &input_mapping_component : _Mappings[_CurrentInputMappingIndex]._Components)
        {
          if (input_note_number == input_mapping_component._MIDINote)
          {
            for (const MappingComponent& output_mapping_component : _Mappings[_CurrentOutputMappingIndex]._Components)
            {
              if (input_mapping_component._Identifier == output_mapping_component._Identifier)
              {
                iplug::IMidiMsg output_message{ message };

                output_message.mData1 = output_mapping_component._MIDINote;

                SendMidiMsg(output_message);

                break;
              }
            }

            break;
          }
        }
      }

      _MidiQueue.Remove();
    }
  }
}

void LiveDrumRemapper::ProcessMidiMsg(const IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}


/*
* Reads the mappings.
*/
void LiveDrumRemapper::ReadMappings() NOEXCEPT
{
  //Clear the mappings.
  _Mappings.Clear();

  //Retrieve the mappings path.
#if APP_API
  DynamicString mappings_path{ RetrievePluginPath("LiveDrumRemapper.exe") };
#elif VST3_API
  DynamicString mappings_path{ RetrievePluginPath("LiveDrumRemapper.vst3") };
#endif

  mappings_path += "LiveDrumRemapperMappings";

  //Iterate over all files in this directory.
  for (const auto &entry : std::filesystem::directory_iterator(std::string(mappings_path.Data())))
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

      new_mapping._MappingName = file_path.substr(start, end - start - 3).c_str();
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

  for (const Mapping& mapping : _Mappings)
  {
    _MappingsMenu.AddItem(mapping._MappingName.Data());
  }
}
#endif