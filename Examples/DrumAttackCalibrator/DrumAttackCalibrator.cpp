//Header file.
#include "DrumAttackCalibrator.h"

//Core.
#include <Core/General/DynamicString.h>

//File.
#undef IN
#undef OUT
#define CATALYST_PLATFORM_WINDOWS
#include <File/Readers/WAVReader.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"

/*
 * Dummy function.
 */
void DummyFunction() {}

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

DrumAttackCalibrator::DrumAttackCalibrator(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{   
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Load the font.
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", DRUMATTACKCALIBRATOR_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };
  };
#endif

  //Load the sound resource.
  {
    //Cache the folder.
    DynamicString file_path{ RetrievePluginPath("DrumAttackCalibrator.vst3") };

    //Add the file name.
    file_path += "\\DrumAttackCalibrator.wav";

    //Read the sound resource.
    if (WAVReader::Read(file_path.Data(), &_SoundResource))
    {
      //Flip the phase of all samples.
      for (DynamicArray<int16> &channel : _SoundResource._Samples)
      {
        for (int16 &sample : channel)
        {
          sample = -sample;
        }
      }
    }
  }
  
}

DrumAttackCalibrator::~DrumAttackCalibrator()
{
  
}

#if IPLUG_DSP
void DrumAttackCalibrator::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  //Cache the number of channels.
  for (int frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > frame_index)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::kNoteOn)
      {
        //Extract the note number and velocity.
        const int note_number{ message.mData1 };

        if (note_number == 24)
        {
          //Add a new sound resource player.
          _SoundResourcePlayers.Emplace();
          SoundResourcePlayer &new_sound_resource_player{ _SoundResourcePlayers.Back() };

          new_sound_resource_player.SetSoundResource(ResourcePointer<SoundResource>(&_SoundResource));
          new_sound_resource_player.SetPlaybackSpeed(_SoundResource._SampleRate / static_cast<float32>(GetSampleRate()));
          new_sound_resource_player.SetGain(0.5'25f);

          for (int channel_index{ 0 }; channel_index < 2; ++channel_index)
          {
            new_sound_resource_player.GetADSREnvelope(channel_index).SetSampleRate(static_cast<float32>(GetSampleRate()));
            new_sound_resource_player.GetADSREnvelope(channel_index).SetStageValues(0.001f, 0.01f, 1.0f, 0.001f);
            new_sound_resource_player.GetADSREnvelope(channel_index).EnterAttackStage();
          }
        }
      }

      _MidiQueue.Remove();
    }

    // Check if any playing sound resource players should be removed.
    bool sound_resource_player_was_removed{false};

    do
    {
      sound_resource_player_was_removed = false;

      for (SoundResourcePlayer &sound_resource_player : _SoundResourcePlayers)
      {
        if (!sound_resource_player.IsActive())
        {
          sound_resource_player = _SoundResourcePlayers.Back();
          _SoundResourcePlayers.Pop();

          sound_resource_player_was_removed = true;

          break;
        }
      }
    } while (sound_resource_player_was_removed);

    for (int channel_index{ 0 }; channel_index < 2; ++channel_index)
    {
      float32 value{ 0.0f };

     for (SoundResourcePlayer &sound_resource_player : _SoundResourcePlayers)
     {
        value += sound_resource_player.NextSample(channel_index);
     }

      outputs[channel_index][frame_index] = value;
    }

    for (int channel_index{ 0 }; channel_index < 2; ++channel_index)
    {
      for (SoundResourcePlayer &sound_resource_player : _SoundResourcePlayers)
      {
        sound_resource_player.Advance(channel_index);
      }
    }
  }
}

void DrumAttackCalibrator::ProcessMidiMsg(const IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}
#endif