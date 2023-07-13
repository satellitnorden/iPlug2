#include "DrumEnginePlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

/*
* Pangaea stuff.
*/
void PangaeaStuff(const char* const RESTRICT piece, const char* const RESTRICT formatted_piece, const char* const RESTRICT microphone, const char* const RESTRICT formatted_microphone) NOEXCEPT
{
  constexpr char* const RESTRICT BASE_DIRECTORY{"C:\\Pangaea Drum Kit Samples"};

  char buffer[MAXIMUM_FILE_PATH_LENGTH];
  sprintf_s(buffer, "%s\\%s", BASE_DIRECTORY, piece);

  uint32 number_of_velocity_layers{0};

  for (const auto& entry : std::filesystem::directory_iterator(std::string(buffer)))
  {
    if (entry.is_directory())
    {
      ++number_of_velocity_layers;
    }
  }

  for (uint32 velocity_layer_index{0}; velocity_layer_index < number_of_velocity_layers; ++velocity_layer_index)
  {
    char buffer[MAXIMUM_FILE_PATH_LENGTH];
    sprintf_s(buffer, "%s\\%s\\VELOCITY LAYER %u\\%s", BASE_DIRECTORY, piece, number_of_velocity_layers - velocity_layer_index, microphone);

    uint32 variation_index{0};

    for (const auto& entry : std::filesystem::directory_iterator(std::string(buffer)))
    {
      char sub_buffer[MAXIMUM_FILE_PATH_LENGTH];
      sprintf_s(sub_buffer, "%s\\%s_%s_%u_%u.wav", BASE_DIRECTORY, formatted_piece, formatted_microphone, velocity_layer_index, variation_index);

      std::filesystem::copy(entry.path(), std::string(sub_buffer), std::filesystem::copy_options::overwrite_existing);

      ++variation_index;
    }
  }
}

/*
* Singularity stuff.
*/
void SingularityStuff(const char* const RESTRICT sub_directory, const char* const RESTRICT piece, const char* const RESTRICT microphone, const char* const RESTRICT formatted_microphone) NOEXCEPT
{
  constexpr char* const RESTRICT BASE_DIRECTORY{"C:\\Singularity Drum Kit Samples"};

  char buffer[MAXIMUM_FILE_PATH_LENGTH];
  sprintf_s(buffer, "%s\\%s", BASE_DIRECTORY, sub_directory);

  uint32 number_of_velocity_layers{ 0 };

  for (const auto &entry : std::filesystem::directory_iterator(std::string(buffer)))
  {
    if (entry.is_directory())
    {
      ++number_of_velocity_layers;
    }
  }

  if (number_of_velocity_layers == 0)
  {
    number_of_velocity_layers = 1;
  }

  for (uint32 velocity_layer_index{ 0 }; velocity_layer_index < number_of_velocity_layers; ++velocity_layer_index)
  {
    char buffer[MAXIMUM_FILE_PATH_LENGTH];

    if (number_of_velocity_layers == 1)
    {
      sprintf_s(buffer, "%s\\%s", BASE_DIRECTORY, sub_directory);
    }

    else
    {
      sprintf_s(buffer, "%s\\%s\\VELOCITY LAYER %u", BASE_DIRECTORY, sub_directory, number_of_velocity_layers - velocity_layer_index);
    }

    uint32 variation_index{ 0 };

    for (const auto &entry : std::filesystem::directory_iterator(std::string(buffer)))
    {
      char sub_buffer[MAXIMUM_FILE_PATH_LENGTH];
      sprintf_s(sub_buffer, "%s\\%s_%s_%u_%u.wav", BASE_DIRECTORY, piece, formatted_microphone, velocity_layer_index, variation_index);

      std::filesystem::copy(entry.path(), std::string(sub_buffer), std::filesystem::copy_options::overwrite_existing);

      ++variation_index;
    }
  }
}

/*
 * Dummy function.
 */
void DummyFunction()
{

}

/*
 * Retrieves the path of this plugin.
 */
DynamicString RetrievePluginPath(const char* const RESTRICT plugin_name) NOEXCEPT
{
  // Retrieve the folder.
  char file_path[MAX_PATH];
  HMODULE module_handle{nullptr};

  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&DummyFunction, &module_handle) == 0)
  {
    return DynamicString();
  }

  if (GetModuleFileNameA(module_handle, file_path, sizeof(file_path)) == 0)
  {
    return DynamicString();
  }

  // Skip the actual plugin name.
  std::string file_path_string{file_path};

  {
    std::string from{std::string("\\") + std::string(plugin_name)};
    size_t start_position{file_path_string.find(from)};

    if (start_position != std::string::npos)
    {
      file_path_string.replace(start_position, from.length(), "\\");
    }
  }

  return DynamicString(file_path_string.c_str());
}

class DrumEnginePluginControl : public IControl
{

public:

  /*
  * Default constructor.
  */
  FORCE_INLINE DrumEnginePluginControl(const iplug::igraphics::IRECT &bounds) NOEXCEPT
    :
    IControl(bounds)
  {

  }

  /*
  * Default destructor.
  */
  FORCE_INLINE virtual ~DrumEnginePluginControl() NOEXCEPT
  {

  }

  /*
  * Callback for when this control need to be rebuilt.
  */
  FORCE_INLINE virtual void Rebuild() NOEXCEPT
  {
    SetDirty();
  }

};

/*
 * Dropdown menu class definition.
 */
class DropDownMenu final : public DrumEnginePluginControl
{

public:

  enum class Function : uint8
  {
    SOUNDBANK,
    MIXER_CHANNEL
  };

  DropDownMenu(DrumEnginePlugin *const RESTRICT plugin, const iplug::igraphics::IRECT &bounds, const Function function, const uint32 extra_data_1 = 0)
    :
    DrumEnginePluginControl(bounds)
  {
    _Plugin = plugin;
    _Function = function;
    _ExtraData1 = extra_data_1;

    CreatePopupMenu();
  }

  /*
  * Callback for when this control need to be rebuilt.
  */
  FORCE_INLINE void Rebuild() NOEXCEPT override
  {
    //Call parent function.
    DrumEnginePluginControl::Rebuild();

    //Do function-specific things.
    switch (_Function)
    {
      case Function::MIXER_CHANNEL:
      {
        CreatePopupMenu();

        break;
      }
    }
  }

  void OnMouseDown(float X, float Y, const IMouseMod& mod) override
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

  enum class HoverState : uint8
  {
    IDLE,
    HOVERED,
    CLICKED
  };

  DrumEnginePlugin *RESTRICT _Plugin;
  Function _Function;
  uint32 _ExtraData1;
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };
  IPopupMenu _PopupMenu;

  void CreatePopupMenu() NOEXCEPT
  {
    _PopupMenu.Clear();

    switch (_Function)
    {
      case Function::SOUNDBANK:
      {
        for (const StaticString<32>& found_soundbank : _Plugin->_DrumEngine.GetFoundSoundBanks())
        {
          _PopupMenu.AddItem(found_soundbank.Data());
        }

        break;
      }

      case Function::MIXER_CHANNEL:
      {
        const DrumEngineUserInterfaceInformation user_interface_information{_Plugin->_DrumEngine.GetUserInterfaceInformation()};

        if (_ExtraData1 < user_interface_information._NumberOfMixerChannels)
        {
          const DrumEngineMixerChannel &mixer_channel{user_interface_information._MixerChannels[_ExtraData1]};

          if (mixer_channel._IsStereo)
          {
            for (uint32 i{ 0 }; i < 63; ++i)
            {
              char buffer[16];
              sprintf_s(buffer, "%u / %u", i + 1, i + 2);

              _PopupMenu.AddItem(buffer);
            }
          }

          else
          {
            for (uint32 i{0}; i < 64; ++i)
            {
              char buffer[16];
              sprintf_s(buffer, "%u", i + 1);

              _PopupMenu.AddItem(buffer);
            }
          }
        }

        break;
      }
    }
  }

  void CalculateName()
  {
    switch (_Function)
    {
      case Function::SOUNDBANK:
      {
        const DrumEngineUserInterfaceInformation user_interface_information{ _Plugin->_DrumEngine.GetUserInterfaceInformation() };

        if (user_interface_information._SoundBankName.Data()[0] != '\0')
        {
          char buffer[128];
          sprintf_s(buffer, "Soundbank: %s", user_interface_information._SoundBankName.Data());

          _Name = buffer;
        }

        else
        {
          _Name = "Soundbank: NONE";
        }

        break;
      }

      case Function::MIXER_CHANNEL:
      {
        const DrumEngineUserInterfaceInformation user_interface_information{ _Plugin->_DrumEngine.GetUserInterfaceInformation() };

        if (_ExtraData1 < user_interface_information._NumberOfMixerChannels)
        {
          const DrumEngineMixerChannel &mixer_channel{ user_interface_information._MixerChannels[_ExtraData1] };
          const uint32 mixer_routing{ _Plugin->_SaveData._Preset._MixerRouting._Routing[_ExtraData1] };

          if (mixer_channel._IsStereo)
          {
            char buffer[128];
            sprintf_s(buffer, "%s: %u / %u", mixer_channel._Name.Data(), mixer_routing + 1, mixer_routing + 2);

            _Name = buffer;
          }

          else
          {
            char buffer[128];
            sprintf_s(buffer, "%s: %u ", mixer_channel._Name.Data(), mixer_routing + 1);

            _Name = buffer;
          }
        }

        else
        {
          _Name = "Empty Channel";
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
    _Plugin->GetUI()->CreatePopupMenu(*this, _PopupMenu, mRECT);

    const int chosen_item_index{ _PopupMenu.GetChosenItemIdx() };

    if (chosen_item_index > -1)
    {
      switch (_Function)
      {
        case Function::SOUNDBANK:
        {
          //Update the preset.
          _Plugin->_SaveData._Preset._SoundBankName = _Plugin->_DrumEngine.GetFoundSoundBanks()[chosen_item_index].Data();
          memset(&_Plugin->_SaveData._Preset._MixerRouting, 0, sizeof(DrumEngineMixerRouting));

          //Apply the preset!
          _Plugin->_DrumEngine.ApplyPreset(_Plugin->_SaveData._Preset);

          break;
        }

        case Function::MIXER_CHANNEL:
        {
          //Update the preset.
          _Plugin->_SaveData._Preset._MixerRouting._Routing[_ExtraData1] = chosen_item_index;

          //Apply the preset!
          _Plugin->_DrumEngine.ApplyPreset(_Plugin->_SaveData._Preset);

          break;
        }
      }
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

DrumEnginePlugin::DrumEnginePlugin(const InstanceInfo& info)
  : Plugin(info, MakeConfig(NUMBER_OF_PLUGIN_PARAMETERS, kNumPresets))
{
  GetParam(CURRENT_SOUNDBANK)->InitInt("Gain", 0, 0, 0, "Soundbank:");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Enable mouse over events.
    pGraphics->EnableMouseOver(true);

    //Load the font.
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the background.
    pGraphics->AttachPanelBackground(IColor(255, 5, 5, 5));

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Add the title.
    pGraphics->AttachControl(new ITextControl(bounds.GetCentredInside(512).GetVShifted(-225), "Drum Engine", IText(50, IColor(255, 225, 225, 225))));

    //Add the soundbank drop down menu.
    {
      DropDownMenu *const RESTRICT control{new DropDownMenu(this, bounds.GetCentredInside(256).GetMidVPadded(16).GetVShifted(-175), DropDownMenu::Function::SOUNDBANK)};
      pGraphics->AttachControl(control);
      _Controls.Emplace(control);
    }

    //Add the mixer channel drop down menus.
    for (uint32 i{ 0 }; i < 32; ++i)
    {
      const uint32 column_index{ i / 8 };
      const uint32 row_index{ i % 8 };
      DropDownMenu *const RESTRICT control{new DropDownMenu(this, bounds.GetCentredInside(96).GetMidVPadded(12).GetHShifted(-400.0f + (112.5f * static_cast<float32>(row_index))).GetVShifted(125.0f + 32.0f * static_cast<float32>(column_index)), DropDownMenu::Function::MIXER_CHANNEL, i)};
      pGraphics->AttachControl(control);
      _Controls.Emplace(control);
    }
  };
#endif

  //Set up the save data.
  memset(&_SaveData, 0, sizeof(DrumEnginePluginSaveData));
  _SaveData._Version = DrumEnginePluginSaveData::CURRENT_VERSION;

  //Set up the drum engine.
  {
    DrumEngineParameters parameters;

#if APP_API
    parameters._WorkingDirectory = "C:\\Users\\Dennis\\My Drive\\Share Folder\\Plugins";
#else
    parameters._WorkingDirectory = RetrievePluginPath("DrumEngine.vst3");
#endif

    _DrumEngine.Initialize(parameters);
  }

  //Set up the "INFINITY" soundbank.
  //SetUpInfinitySoundBank(true, false);

  //Set up the "PANGAEA" soundbank.
  //SetUpPangaeaSoundBank(true, false);

  //Set up the "SINGULARITY" soundbank.
  SetUpSingularitySoundBank(true, false);

  //PangaeaStuff("KICK", "KICK", "OVERHEAD", "OVERHEAD");
  //PangaeaStuff("SNARE", "SNARE", "BLUE MOUSE", "BLUE_MOUSE");
  //PangaeaStuff("SNARE", "SNARE", "MD421", "MD421");
  //PangaeaStuff("SNARE", "SNARE", "OVERHEAD", "OVERHEAD");
  //PangaeaStuff("TOM 1", "TOM_1", "D112", "D112");
  //PangaeaStuff("TOM 1", "TOM_1", "MD421", "MD421");
  //PangaeaStuff("TOM 1", "TOM_1", "OVERHEAD", "OVERHEAD");
  //PangaeaStuff("TOM 2", "TOM_2", "D112", "D112");
  //PangaeaStuff("TOM 2", "TOM_2", "MD421", "MD421");
  //PangaeaStuff("TOM 2", "TOM_2", "OVERHEAD", "OVERHEAD");
  //PangaeaStuff("TOM 3", "TOM_3", "D112", "D112");
  //PangaeaStuff("TOM 3", "TOM_3", "MD421", "MD421");
  //PangaeaStuff("TOM 3", "TOM_3", "OVERHEAD", "OVERHEAD");
  //PangaeaStuff("CHINA", "CHINA", "BLUE MOUSE", "BLUE_MOUSE");
  //PangaeaStuff("RIGHT CRASH", "RIGHT_CRASH", "BLUE MOUSE", "BLUE_MOUSE");
  //PangaeaStuff("STACK", "STACK", "BLUE MOUSE", "BLUE_MOUSE");
  //PangaeaStuff("LEFT CRASH", "LEFT_CRASH", "BLUE MOUSE", "BLUE_MOUSE");

  //SingularityStuff("KICK\\D112", "KICK", "D112", "D112");
  //SingularityStuff("KICK\\BETA 52", "KICK", "BETA 52", "BETA_52");
  //SingularityStuff("KICK\\ROOM", "KICK", "ROOM", "ROOM");

  //SingularityStuff("SNARE\\SM57", "SNARE", "SM57", "SM57");
  //SingularityStuff("SNARE\\MD421", "SNARE", "MD421", "MD421");
  //SingularityStuff("SNARE\\OVERHEAD", "SNARE", "OVERHEAD", "OVERHEAD");
  //SingularityStuff("SNARE\\ROOM", "SNARE", "ROOM", "ROOM");

#if DEBUG_PLUGIN
  DrumEnginePreset preset;

  memset(&preset, 0, sizeof(DrumEnginePreset));

  preset._Name = "DEBUG";
  preset._SoundBankName = "PANGAEA";
  preset._MixerRouting._Routing[0] = 2;
  preset._MixerRouting._Routing[1] = 3;

  _DrumEngine.ApplyPreset(preset);
#endif
}

void DrumEnginePlugin::OnIdle()
{
  if (_RebuildUI)
  {
    for (DrumEnginePluginControl *const RESTRICT control : _Controls)
    {
      control->Rebuild();
    }

    _RebuildUI = false;
  }
}

#if IPLUG_DSP
void DrumEnginePlugin::ProcessMidiMsg(const iplug::IMidiMsg &msg)
{
  _MidiQueue.Add(msg);
}

void DrumEnginePlugin::ProcessBlock(sample **inputs, sample **outputs, int number_of_frames)
{
#if DEBUG_PLUGIN
  static uint32 FAKE_PLAYING_COUNTER{ 0 };
#endif

  const DrumEngineUpdateState update_state{ _DrumEngine.Update() };

  if (update_state != DrumEngineUpdateState::NONE)
  {
    _RebuildUI = true;
  }

  const uint32 number_of_channels{ static_cast<uint32>(NOutChansConnected()) };
  
  for (uint32 frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
#if DEBUG_PLUGIN
    {
      if ((FAKE_PLAYING_COUNTER % 44100) == 0)
      {
        _DrumEngine.OnNoteOn(24, 127);
      }

      ++FAKE_PLAYING_COUNTER;
    }
#endif

    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg& message{_MidiQueue.Peek()};

      if (message.mOffset > frame_index)
      {
        break;
      }

      if (message.StatusMsg() == iplug::IMidiMsg::kNoteOn)
      {
        _DrumEngine.OnNoteOn(message.mData1, message.mData2);
      }

      _MidiQueue.Remove();
    }

    float32 samples[64];
    memset(samples, 0, sizeof(float32) * 64);

    _DrumEngine.ProcessSample(number_of_channels, samples);

    for (uint32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      outputs[channel_index][frame_index] = samples[channel_index];
    }
  }
}
#endif

bool DrumEnginePlugin::SerializeState(iplug::IByteChunk &chunk) const
{
  const bool parent_succeeded{ iplug::Plugin::SerializeState(chunk) };

  chunk.PutBytes(&_SaveData, sizeof(DrumEnginePluginSaveData));

  return parent_succeeded;
}

int DrumEnginePlugin::UnserializeState(const iplug::IByteChunk &chunk, int start_position)
{
  int32 current_position{ iplug::Plugin::UnserializeState(chunk, start_position) };

  uint64 version{ 0 };
  chunk.GetBytes(&version, sizeof(uint64), current_position);

  current_position += sizeof(uint64);

  bool correctly_loaded{ false };

  switch (version)
  {
    case DrumEnginePluginSaveData::CURRENT_VERSION:
    {
      //This is the current version, so just copy the rest!
      constexpr uint64 BYTES_TO_READ{ sizeof(DrumEnginePluginSaveData) - sizeof(uint64) };
      _SaveData._Version = version;
    
      chunk.GetBytes(((byte*)&_SaveData) + sizeof(uint64), BYTES_TO_READ, current_position);

      current_position += BYTES_TO_READ;

      correctly_loaded = true;

      break;
    }

    default:
    {
      ASSERT(false, "Invalid case!");

      break;
    }
  }

  if (correctly_loaded)
  {
      //Now that save data has been loaded, load the soundbank. (:
      if (_SaveData._Preset._SoundBankName.Data()[0] != '\0')
      {
        _DrumEngine.ApplyPreset(_SaveData._Preset);
      }
  }

  return current_position;
}

/*
* Sets up the "INFINITY" soundbank.
*/
void DrumEnginePlugin::SetUpInfinitySoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT
{
  constexpr uint32 NUMBER_OF_VELOCITY_LAYERS{8};
  constexpr uint32 NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER{8};

  DrumEngineSoundBankCreationParameters parameters;

  parameters._Name = "INFINITY";

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGEST;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_1";
    new_piece_information._MIDINote = 35;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 2;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_2";
    new_piece_information._MIDINote = 36;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 2;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_3";
    new_piece_information._MIDINote = 37;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 2;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_4";
    new_piece_information._MIDINote = 38;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 2;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "CHINA";
    new_piece_information._MIDINote = 67;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_BOW";
    new_piece_information._MIDINote = 62;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_BELL";
    new_piece_information._MIDINote = 63;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_CRASH";
    new_piece_information._MIDINote = 64;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIGHT_CRASH";
    new_piece_information._MIDINote = 54;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "STACK";
    new_piece_information._MIDINote = 78;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SPLASH";
    new_piece_information._MIDINote = 73;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "LEFT_CRASH";
    new_piece_information._MIDINote = 52;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_0";
    new_piece_information._MIDINote = 40;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_1";
    new_piece_information._MIDINote = 41;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_2";
    new_piece_information._MIDINote = 42;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_3";
    new_piece_information._MIDINote = 43;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_4";
    new_piece_information._MIDINote = 44;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_5";
    new_piece_information._MIDINote = 45;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_6";
    new_piece_information._MIDINote = 46;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_7";
    new_piece_information._MIDINote = 47;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_PEDAL";
    new_piece_information._MIDINote = 48;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_SPLASH";
    new_piece_information._MIDINote = 49;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "WIDE_LEFT_CRASH";
    new_piece_information._MIDINote = 56;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 4;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{ parameters._MixerChannels.Back() };

    new_mixer_channel._Name = "MAIN";
    new_mixer_channel._IsStereo = true;
  }

  parameters._InputDirectory = "C:\\Users\\Dennis\\Documents\\Reaper Projects\\Superior Drummer HD Mix\\INFINITY SOUNDBANK";
  parameters._OutputDirectory = "C:\\Users\\Dennis\\My Drive\\Share Folder\\Plugins\\DrumEngineSoundBanks";

  //Create the soundbank.
  if (create_soundbank)
  {
    _DrumEngine.CreateSoundBank(parameters);
  }

  //Export regions/MIDI.
  if (create_regions_and_midis)
  {
    _DrumEngine.ExportRegionsAndMIDIForSoundBank(parameters, "Regions and MIDIs");
  }
}

/*
* Sets up the "SINGULARITY" soundbank.
*/
void DrumEnginePlugin::SetUpSingularitySoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT
{
  constexpr uint32 OVERHEAD_MIXER_CHANNEL_INDEX{ 11 };

  DrumEngineSoundBankCreationParameters parameters;

  parameters._Name = "SINGULARITY";

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation &new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_D112";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 1;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation &new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_BETA_52";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 1;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 1;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_ROOM";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 1;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 2;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_SM57";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 2;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 3;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_MD421";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 2;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_ROOM";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 2;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 5;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_OVERHEAD";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 2;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 6;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{ parameters._MixerChannels.Back() };

    new_mixer_channel._Name = "KICK D112";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{ parameters._MixerChannels.Back() };

    new_mixer_channel._Name = "KICK BETA 52";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel  new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "KICK ROOM";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE SM57";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE MD421";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE ROOM";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel &new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "OVERHEADS";
    new_mixer_channel._IsStereo = true;
  }

  parameters._InputDirectory = "C:\\Singularity Drum Kit Samples";
  parameters._OutputDirectory = "C:\\Users\\Dennis\\My Drive\\Share Folder\\Plugins\\DrumEngineSoundBanks";

  // Create the soundbank.
  if (create_soundbank)
  {
    _DrumEngine.CreateSoundBank(parameters);
  }

  // Export regions/MIDI.
  if (create_regions_and_midis)
  {
    _DrumEngine.ExportRegionsAndMIDIForSoundBank(parameters, "Regions and MIDIs");
  }
}

/*
 * Sets up the "PANGAEA" soundbank.
 */
void DrumEnginePlugin::SetUpPangaeaSoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT
{
  constexpr float32 CYMBALS_PANNING{0.5f};
  constexpr uint32 OVERHEAD_MIXER_CHANNEL_INDEX{11};

  DrumEngineSoundBankCreationParameters parameters;

  parameters._Name = "PANGAEA";

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_D112";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 5;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 0;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_BETA_52";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 5;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 1;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "KICK_OVERHEAD";
    new_piece_information._MIDINote = 24;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 5;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_SM57";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 6;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 2;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_MD421";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 6;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 3;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_BLUE_MOUSE";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 6;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE_OVERHEAD";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 6;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_1_D112";
    new_piece_information._MIDINote = 35;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._Panning = 0.0f;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 5;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_1_MD421";
    new_piece_information._MIDINote = 35;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 6;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_1_OVERHEAD";
    new_piece_information._MIDINote = 35;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_2_D112";
    new_piece_information._MIDINote = 36;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 5;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_2_MD421";
    new_piece_information._MIDINote = 36;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 7;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_2_OVERHEAD";
    new_piece_information._MIDINote = 36;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_3_D112";
    new_piece_information._MIDINote = 37;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 8;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_3_MD421";
    new_piece_information._MIDINote = 37;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = 9;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_3_OVERHEAD";
    new_piece_information._MIDINote = 37;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "CHINA_BLUE_MOUSE";
    new_piece_information._MIDINote = 67;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = CYMBALS_PANNING * (3.0f / 3.0f);
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIGHT_CRASH_BLUE_MOUSE";
    new_piece_information._MIDINote = 54;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = CYMBALS_PANNING * (1.0f / 3.0f);
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "STACK_BLUE_MOUSE";
    new_piece_information._MIDINote = 78;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = 0.0f;
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "LEFT_CRASH_BLUE_MOUSE";
    new_piece_information._MIDINote = 52;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = 4;
    new_piece_information._NumberOfSamplesPerVelocityLayer = 16;
    new_piece_information._Panning = -CYMBALS_PANNING * (1.0f / 3.0f);
    new_piece_information._Length = 1;
    new_piece_information._MixerChannelIndex = OVERHEAD_MIXER_CHANNEL_INDEX;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "KICK D112";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "KICK BETA 52";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE SM57";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE MD421";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "SNARE BLUE MOUSE";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 1 D112";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 1 MD421";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 2 D112";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 2 MD421";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 3 D112";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "TOM 3 MD421";
    new_mixer_channel._IsStereo = false;
  }

  {
    parameters._MixerChannels.Emplace();
    DrumEngineMixerChannel& new_mixer_channel{parameters._MixerChannels.Back()};

    new_mixer_channel._Name = "OVERHEADS";
    new_mixer_channel._IsStereo = true;
  }

  parameters._InputDirectory = "C:\\Pangaea Drum Kit Samples";
  parameters._OutputDirectory = "C:\\Users\\Dennis\\My Drive\\Share Folder\\Plugins\\DrumEngineSoundBanks";

  // Create the soundbank.
  if (create_soundbank)
  {
    _DrumEngine.CreateSoundBank(parameters);
  }

  // Export regions/MIDI.
  if (create_regions_and_midis)
  {
    _DrumEngine.ExportRegionsAndMIDIForSoundBank(parameters, "Regions and MIDIs");
  }
}