#include "DrumEnginePlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

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

/*
 * Dropdown menu class definition.
 */
class DropDownMenu final : public IControl
{

public:

  enum class Function : uint8
  {
    SOUNDBANK
  };

  DropDownMenu(DrumEnginePlugin* const RESTRICT plugin, const iplug::igraphics::IRECT &bounds, const Function function)
    : IControl(bounds)
  {
    _Plugin = plugin;
    _Function = function;

    CreatePopupMenu();
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

  void Draw(IGraphics& graphics) override
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
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };
  IPopupMenu _PopupMenu;

  void CreatePopupMenu() NOEXCEPT
  {
    for (const StaticString<32> &found_soundbank : _Plugin->_DrumEngine.GetFoundSoundBanks())
    {
      _PopupMenu.AddItem(found_soundbank.Data());
    }
  }

  void CalculateName()
  {
    switch (_Function)
    {
      case Function::SOUNDBANK:
      {
        char buffer[128];
        sprintf_s(buffer, "Soundbank: %s", _Plugin->_DrumEngine.GetCurrentSoundBank()->_Name.Data());

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
    _Plugin->GetUI()->CreatePopupMenu(*this, _PopupMenu, mRECT);

    const int chosen_item_index{ _PopupMenu.GetChosenItemIdx() };

    if (chosen_item_index > -1)
    {
      //Let the drum engine know that we want to switch.
      _Plugin->_DrumEngine.SetSoundBankSwitchState(SoundBankSwitchState::WANTS_TO_SWITCH);

      //Wait for the engine to be ready.
      while (_Plugin->_DrumEngine.GetSoundBankSwitchState() != SoundBankSwitchState::READY_TO_SWITCH);

      //Load the new soundbank!
      _Plugin->_DrumEngine.LoadSoundBank(_Plugin->_DrumEngine.GetFoundSoundBanks()[chosen_item_index].Data());

      //Signal that we have switched. (:
      _Plugin->_DrumEngine.SetSoundBankSwitchState(SoundBankSwitchState::HAS_SWITCHED);
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
    pGraphics->AttachPanelBackground(IColor(255, 25, 25, 25));

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Add the title.
    pGraphics->AttachControl(new ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Drum Engine", IText(50)));

    //Add the soundbank drop down menu.
    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(256).GetMidVPadded(24), DropDownMenu::Function::SOUNDBANK));
  };
#endif

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

  //Load the "INFINITY" soundbank.
  _DrumEngine.LoadSoundBank("INFINITY");
}

#if IPLUG_DSP
void DrumEnginePlugin::ProcessMidiMsg(const iplug::IMidiMsg &msg)
{
  _MidiQueue.Add(msg);
}

#define FAKE_PLAYING (0)

void DrumEnginePlugin::ProcessBlock(sample **inputs, sample **outputs, int number_of_frames)
{
#if FAKE_PLAYING
  static uint32 FAKE_PLAYING_COUNTER{ 0 };
#endif

  const uint32 number_of_channels{ static_cast<uint32>(NOutChansConnected()) };
  
  for (uint32 frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
#if FAKE_PLAYING
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

    float32 samples[2];

    _DrumEngine.ProcessSample(samples);

    for (uint32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      outputs[channel_index][frame_index] = samples[channel_index];
    }
  }
}
#endif


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
    new_piece_information._Length = 1;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SNARE";
    new_piece_information._MIDINote = 26;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
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
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_2";
    new_piece_information._MIDINote = 36;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 2;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_3";
    new_piece_information._MIDINote = 37;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 2;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "TOM_4";
    new_piece_information._MIDINote = 38;
    new_piece_information._VelocityCurve = VelocityCurve::STRONGER;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 2;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "CHINA";
    new_piece_information._MIDINote = 67;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_BOW";
    new_piece_information._MIDINote = 62;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_BELL";
    new_piece_information._MIDINote = 63;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIDE_CRASH";
    new_piece_information._MIDINote = 64;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "RIGHT_CRASH";
    new_piece_information._MIDINote = 54;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "STACK";
    new_piece_information._MIDINote = 78;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "SPLASH";
    new_piece_information._MIDINote = 73;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "LEFT_CRASH";
    new_piece_information._MIDINote = 52;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_0";
    new_piece_information._MIDINote = 40;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_1";
    new_piece_information._MIDINote = 41;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_2";
    new_piece_information._MIDINote = 42;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_3";
    new_piece_information._MIDINote = 43;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_4";
    new_piece_information._MIDINote = 44;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_5";
    new_piece_information._MIDINote = 45;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_6";
    new_piece_information._MIDINote = 46;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_LEVEL_7";
    new_piece_information._MIDINote = 47;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_PEDAL";
    new_piece_information._MIDINote = 48;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "HIHAT_SPLASH";
    new_piece_information._MIDINote = 49;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
  }

  {
    parameters._PieceInformations.Emplace();
    DrumEngineSoundBankCreationPieceInformation& new_piece_information{parameters._PieceInformations.Back()};

    new_piece_information._Name = "WIDE_LEFT_CRASH";
    new_piece_information._MIDINote = 56;
    new_piece_information._VelocityCurve = VelocityCurve::LINEAR;
    new_piece_information._NumberOfVelocityLayers = NUMBER_OF_VELOCITY_LAYERS;
    new_piece_information._NumberOfSamplesPerVelocityLayer = NUMBER_OF_SAMPLES_PER_VELOCITY_LAYER;
    new_piece_information._Length = 4;
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
* Sets up the "PANGAEA" soundbank.
*/
void DrumEnginePlugin::SetUpPangaeaSoundBank(const bool create_soundbank, const bool create_regions_and_midis) NOEXCEPT
{
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
    new_piece_information._Length = 1;
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