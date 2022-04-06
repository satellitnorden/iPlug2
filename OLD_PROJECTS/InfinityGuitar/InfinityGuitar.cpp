//Header file.
#include "InfinityGuitar.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//Core.
#include <Core/General/DynamicString.h>

//File.
#undef IN
#undef OUT
#define CATALYST_PLATFORM_WINDOWS
#include <File/Core/BinaryFile.h>
#include <File/Readers/WAVReader.h>

//Math.
#include <Math/Core/CatalystRandomMath.h>

//Windows.
#include <ShlObj_core.h>
#include <filesystem>

//InfinityGuitar constants.
namespace InfinityGuitarConstants
{
  constexpr float32 MAXIMUM_HUMANIZE_GAIN{ 0.1f }; //0.025f step.
  constexpr float32 MAXIMUM_HUMANIZE_RELEASE{ 0.005f }; //0.025f step.
  constexpr float32 MAXIMUM_HUMANIZE_PLAYBACK_SPEED{ 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f };
  constexpr float32 MAXIMUM_HUMANIZE_OFFSET{ 4.0f };
  constexpr float32 MAXIMUM_HUMANIZE_STARTING_POSITION{ 0.00'25f };
  constexpr float32 OPEN_TAPPED_NOTES_STARTING_POSITION{ 0.1'00f }; //0.025f step.
  constexpr float32 MUTED_TAPPED_NOTES_STARTING_POSITION{ 0.0'25f }; //0.025f step.
  StaticArray<float32, 64> CONSTANT_POWER_LOOKUP;
}

//Infinity guitar logic.
namespace InfinityGuitarLogic
{

  /*
  * Returns the string for a certain type.
  */
  FORCE_INLINE RESTRICTED NO_DISCARD const char* const RESTRICT GetTypeString(const InfinityGuitar::Type type) NOEXCEPT
  {
    switch (type)
    {
    case InfinityGuitar::Type::DI:
    {
      return "DI";
    }

    case InfinityGuitar::Type::RHYTHM:
    {
      return "RHYTHM";
    }

    case InfinityGuitar::Type::LEAD:
    {
      return "LEAD";
    }

    case InfinityGuitar::Type::CLEAN:
    {
      return "CLEAN";
    }

    default:
    {
      ASSERT(false, "Invalid case!");

      return "";
    }
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
DynamicString RetrievePluginPath(const char *const RESTRICT plugin_name) NOEXCEPT
{
  //Retrieve the folder.
  char file_path[MAX_PATH];
  HMODULE module_handle{ nullptr };

  if (GetModuleHandleExA(  GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    (LPCSTR) &DummyFunction,
    &module_handle) == 0)
  {
    return DynamicString();
  }

  if (GetModuleFileNameA(module_handle, file_path, sizeof(file_path)) == 0)
  {
    return DynamicString();
  }

  //Skip the actual plugin name.
  std::string file_path_string{ file_path };

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

InfinityGuitar::InfinityGuitar(const iplug::InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Set up the constant power lookup.
  for (uint64 i{ 0 }, size{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size() }; i < size; ++i)
  {
    InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[i] = CatalystBaseMath::SquareRoot(static_cast<float32>(i + 1)) / static_cast<float32>(i + 1);
  }

  GetParam(CHANNEL_PARAM)->InitInt("CHANNEL", 0, 0, 1);
  GetParam(TYPE_PARAM)->InitInt("TYPE", 0, 0, 3);
  GetParam(HUMANIZE_PARAM)->InitDouble("HUMANIZE", 100.0, 0.0, 100.0, 0.01);

#if IPLUG_EDITOR
  //Set the make graphics function.
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  //Set the layout function.
  mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics)
  {
    //Load the font.
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", INFINITYGUITAR_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachBackground(INFINITYGUITAR_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the info text.
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Open Notes: 100-127", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-96), "Muted Notes: 90-99", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-64), "Tapped Open Notes: 80-89", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-32), "Tapped Muted notes : 70-79", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(0), "Dead notes : 60-69", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));

    //Create the row of knobs.
    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      _ChannelKnob = new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(-128), CHANNEL_PARAM, "Channel", style);

      pGraphics->AttachControl(_ChannelKnob);

      GetParam(CHANNEL_PARAM)->SetDisplayFunc([this](const double, WDL_String& string)
      {
        string.Set(_CurrentChannel == Channel::LEFT ? "LEFT" : "RIGHT");
      });
    }

    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      _TypeKnob = new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128), TYPE_PARAM, "Type", style);

      pGraphics->AttachControl(_TypeKnob);

      GetParam(TYPE_PARAM)->SetDisplayFunc([this](const double, WDL_String& string)
      {
        string.Set(InfinityGuitarLogic::GetTypeString(_CurrentType));
      });
    }

    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      _HumanizeKnob = new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(128), HUMANIZE_PARAM, "Humanize", style);

      pGraphics->AttachControl(_HumanizeKnob);

      GetParam(HUMANIZE_PARAM)->SetDisplayFunc([this](const double value, WDL_String& string)
      {
        char buffer[64];
        sprintf_s(buffer, "%i%%", static_cast<int32>(value));

        string.Set(buffer);
      });
    }
  };
#endif

  //Export packages.
  //ExportPackages();

  //Import packages.
  ImportPackages(_CurrentChannel, _CurrentType);

#if USE_OUTPUT_LOG
  //Open the output log.
  char buffer[MAX_PATH];
  sprintf_s(buffer, "Infinity Guitar Output Log %i.txt", CatalystRandomMath::RandomIntegerInRange<int32>(0, INT32_MAXIMUM));

  _OutputLog.open(buffer);
#endif
}

InfinityGuitar::~InfinityGuitar()
{
#if USE_OUTPUT_LOG
  //Close the output log.
  _OutputLog.close();
#endif
}

#if IPLUG_DSP
void InfinityGuitar::OnReset()
{
  for (PlayingNote& playing_note : _PlayingNotes)
  {
    playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
  }
}

void InfinityGuitar::OnParamChange(const int32 index)
{
  switch (index)
  {
  case CHANNEL_PARAM:
  {
    const Channel new_channel{ static_cast<Channel>(GetParam(CHANNEL_PARAM)->Int() + 1) };

    if (_CurrentChannel != new_channel)
    {
      _CurrentChannel = new_channel;

      ImportPackages(_CurrentChannel, _CurrentType);
    }

#if USE_OUTPUT_LOG
    //_OutputLog << "OnParamChange - Channel - " << static_cast<int32>(new_channel) << std::endl;
#endif

    break;
  }

  case TYPE_PARAM:
  {
    const Type new_type{ static_cast<Type>(GetParam(TYPE_PARAM)->Int() + 1) };

    if (_CurrentType != new_type)
    {
      _CurrentType = new_type;

      ImportPackages(_CurrentChannel, _CurrentType);
    }

#if USE_OUTPUT_LOG
    //_OutputLog << "OnParamChange - Type - " << static_cast<int32>(new_type) << std::endl;
#endif

    break;
  }

  case HUMANIZE_PARAM:
  {
    _HumanizeFactor = static_cast<float>(GetParam(HUMANIZE_PARAM)->Value() / 100.0);

    break;
  }
  }
}

void InfinityGuitar::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  //Nothing to process if there are no notes.
  if (_Notes.Empty())
  {
    return;
  }

  //If the plugin wants to reload notes, signal that the plugin is ready to reload notes and return.
  if (_WantsToReloadNotes.IsSet())
  {
    if (!_IsReadyToToReloadNotes.IsSet())
    {
      _IsReadyToToReloadNotes.Set();
    }

    return;
  }

  //Otherwise, the plugin is not ready to reload notes.
  else
  {
    if (_IsReadyToToReloadNotes.IsSet())
    {
      _IsReadyToToReloadNotes.Clear();
    }
  }

  const int32 number_of_channels{ NOutChansConnected() };

  for (int32 i{ 0 }; i < number_of_frames; ++i)
  {
    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg& message{ _MidiQueue.Peek() };

      if (message.mOffset > i)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::kNoteOn)
      {
        //Extract the note number and velocity.
        const int32 note_number{ message.mData1 };
        const int32 velocity{ CatalystBaseMath::Maximum<int32>(message.mData2, 60) };

        //Determine the note index.
        const int32 note_index{ note_number - 40 };

        if (note_index >= 0 && note_index < _Notes.Size())
        {
          _PlayingNotes.Emplace();
          PlayingNote& playing_note{ _PlayingNotes.Back() };

          SoundResource* RESTRICT sound_resource;

          //Open notes.
          if (velocity >= 100)
          {
            sound_resource = &_Notes[note_index]._OpenNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor);
          }

          //Muted notes.
          else if (velocity >= 90)
          {
            sound_resource = &_Notes[note_index]._MutedNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor);
          }

          /*
          * Tapped open notes/hammer on open notes/hammer off open notes.
          * This is faked by just playing the open notes, but offset the starting sample position a bit.
          */
          else if (velocity >= 80)
          {
            sound_resource = &_Notes[note_index]._OpenNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * (InfinityGuitarConstants::OPEN_TAPPED_NOTES_STARTING_POSITION + CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor));
          }

          /*
          * Tapped muted notes/hammer on muted notes/hammer off muted notes.
          * This is faked by just playing the open notes, but offset the starting sample position a bit.
          */
          else if (velocity >= 70)
          {
            sound_resource = &_Notes[note_index]._MutedNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * (InfinityGuitarConstants::MUTED_TAPPED_NOTES_STARTING_POSITION + CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor));
          }

          /*
          * Dead notes.
          */
          else if (velocity >= 60)
          {
            sound_resource = &_DeadNotes[CatalystBaseMath::Minimum<int32>(note_index / 7, 7)][_DeadNoteRandomIndexer.Next()];
          }

          playing_note._SoundResourcePlayer.SetSoundResource(ResourcePointer<SoundResource>(sound_resource));
          playing_note._SoundResourcePlayer.SetPlaybackSpeed(sound_resource->_SampleRate /  static_cast<float32>(GetSampleRate()) + CatalystRandomMath::RandomFloatInRange(-InfinityGuitarConstants::MAXIMUM_HUMANIZE_PLAYBACK_SPEED, InfinityGuitarConstants::MAXIMUM_HUMANIZE_PLAYBACK_SPEED) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.SetGain(1.0f + CatalystRandomMath::RandomFloatInRange(-InfinityGuitarConstants::MAXIMUM_HUMANIZE_GAIN, InfinityGuitarConstants::MAXIMUM_HUMANIZE_GAIN) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues( 0.001f,
            0.01f,
            1.0f,
            0.01f + CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_RELEASE) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.GetADSREnvelope().EnterAttackStage();
          playing_note._NoteNumber = message.mData1;
        }
      }

      else if (status == iplug::IMidiMsg::kNoteOff)
      {
        for (PlayingNote& playing_note : _PlayingNotes)
        {
          if (playing_note._NoteNumber == message.mData1)
          {
            playing_note._SoundResourcePlayer.GetADSREnvelope().EnterReleaseStage();
          }
        }
      }

      _MidiQueue.Remove();
    }

    //Check if any playing notes should be removed.
    bool note_was_removed{ false };

    do
    {
      note_was_removed = false;

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        if (!playing_note._SoundResourcePlayer.IsActive())
        {
          playing_note = _PlayingNotes.Back();
          _PlayingNotes.Pop();

          note_was_removed = true;

          break;
        }
      }
    } while (note_was_removed);

    //Apply constant power gain.
    {
#if 0
      const uint64 number_of_playing_notes{ _PlayingNotes.Size() };
      const float32 constant_power_gain{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[number_of_playing_notes - 1] };

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        playing_note._SoundResourcePlayer.SetGain(CatalystBaseMath::LinearlyInterpolate(1.0f, constant_power_gain, _CurrentType == Type::DI ? 0.5f : 0.5f));
      }
#else
      float32 number_of_playing_notes{ 0.0f };

      for (const PlayingNote& playing_note : _PlayingNotes)
      {
        number_of_playing_notes += playing_note._SoundResourcePlayer.GetADSREnvelope().CalculateCurrentMultiplier();
      }

      const float32 constant_power_gain_1{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes), InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size() - 1)] };
      const float32 constant_power_gain_2{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes) + 1, InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size() - 1)] };

      const float32 alpha{ CatalystBaseMath::Fractional(number_of_playing_notes) };

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        playing_note._SoundResourcePlayer.SetGain(CatalystBaseMath::LinearlyInterpolate(constant_power_gain_1, constant_power_gain_2, alpha));
      }
#endif
    }

    for (int j{ 0 }; j < number_of_channels; ++j)
    {
      float value{ 0.0f };

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        value += playing_note._SoundResourcePlayer.NextSample(j);
      }

      outputs[j][i] = value;
    }

    for (PlayingNote& playing_note : _PlayingNotes)
    {
      playing_note._SoundResourcePlayer.Advance();
    }
  }
}

void InfinityGuitar::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  iplug::IMidiMsg offset_message{ msg };
  //offset_message.mOffset += CatalystBaseMath::Round<int32>(CatalystRandomMath::RandomFloatInRange(-InfinityGuitarConstants::MAXIMUM_HUMANIZE_OFFSET, InfinityGuitarConstants::MAXIMUM_HUMANIZE_OFFSET) * _HumanizeFactor);

  _MidiQueue.Add(offset_message);
}
#endif

/*
* Exports the packages.
*/
void InfinityGuitar::ExportPackages() NOEXCEPT
{
  //Define constants.
  constexpr char* OUTPUT_FOLDER{ "C:\\Github\\Audio-Data\\Plugins\\Instruments\\InfinityGuitarSamples" };
  constexpr char* DI_FOLDER{ "C:\\Github\\Audio-Data\\Plugins\\Instruments\\InfinityGuitarSamples" };
  constexpr char* RHYTHM_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Djent\\Sounds\\Guitar\\Rhythm" };
  constexpr char* LEAD_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Sounds\\Guitar\\Lead" };
  constexpr char* CLEAN_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Sounds\\Guitar\\Clean" };
  constexpr uint64 MAXIMUM_FILE_SIZE{ 100'000'000 };

  //Iterate over all types, di and dist.
  for (uint8 type_index{ 0 }; type_index < 4; ++type_index)
  {
    //Iterate over both channels.
    for (uint8 channel_index{ 0 }; channel_index < 2; ++channel_index)
    {
      //Cache the channel.
      const Channel channel{ static_cast<Channel>(channel_index + 1) };

      //Cache the channel string.
      const char* channel_string{ channel == Channel::LEFT ? "LEFT" : "RIGHT" };

      //Open the first package file.
      uint32 package_counter{ 0 };
      BinaryFile<BinaryFileMode::OUT> *RESTRICT package_file{ nullptr };

      {
        char package_file_buffer[MAX_PATH];
        sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", OUTPUT_FOLDER, InfinityGuitarLogic::GetTypeString(static_cast<Type>(type_index + 1)), channel_string, ++package_counter);

        package_file = new BinaryFile<BinaryFileMode::OUT>(package_file_buffer);
      }

      //Export all packages.
      char sound_file_buffer[MAX_PATH];
      uint32 octave_counter{ 0 };
      uint32 note_counter{ 0 };
      SoundResource sound_resource;
      uint64 current_file_size{ 0 };
      uint32 dead_note_string_counter{ 0 };

      //Read the regular notes.
      for (;;)
      {
        //Remember the numer of files read.
        uint32 number_of_files_read{ 0 };

        //Read the different types of notes.
        for (uint8 i{ 0 }; i < 2; ++i)
        {
          //Cache the note string.
          const char* note_string{ i == 0 ? "OPEN" : "MUTED" };

          //Read the notes.
          uint32 variation_counter{ 1 };

          //Calculate the type folder/string.
          const char* RESTRICT type_folder;
          const char* RESTRICT type_string;

          switch (type_index)
          {
          case 0:
          {
            type_folder = DI_FOLDER;
            type_string = "DI";

            break;
          }

          case 1:
          {
            type_folder = RHYTHM_FOLDER;
            type_string = "RHYTHM_GUITAR";

            break;
          }

          case 2:
          {
            type_folder = LEAD_FOLDER;
            type_string = "LEAD_GUITAR";

            break;
          }

          case 3:
          {
            type_folder = CLEAN_FOLDER;
            type_string = "CLEAN_GUITAR";

            break;
          }
          }

          for (;;)
          {
            sprintf(sound_file_buffer, "%s\\%s_%s_%u_%u_%s_%u.wav", type_folder, type_string, channel == Channel::LEFT ? "LEFT" : "RIGHT", octave_counter, note_counter, note_string, variation_counter++);

            sound_resource._Samples.Clear();

            if (!WAVReader::Read(sound_file_buffer, &sound_resource))
            {
              break;
            }

            else
            {
              ++number_of_files_read;

              //Calculate the sound file size.
              uint64 sound_file_size{ 0 };

              sound_file_size += sizeof(uint8);
              sound_file_size += sizeof(uint64);
              sound_file_size += sizeof(float32);
              sound_file_size += sizeof(uint8);

              for (const DynamicArray<int16> &channel_samples : sound_resource._Samples)
              {
                sound_file_size += sizeof(int16) * channel_samples.Size();
              }

              //If the file size is too big, open the next package file.
              if (current_file_size + sound_file_size >= MAXIMUM_FILE_SIZE)
              {
                package_file->Close();
                delete package_file;

                char package_file_buffer[MAX_PATH];
                sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", OUTPUT_FOLDER, InfinityGuitarLogic::GetTypeString(static_cast<Type>(type_index + 1)), channel_string, ++package_counter);

                package_file = new BinaryFile<BinaryFileMode::OUT>(package_file_buffer);

                current_file_size = 0;
              }

              //Write the sound file to the file.
              package_file->Write(&sound_resource._SampleRate, sizeof(float32));
              package_file->Write(&sound_resource._NumberOfChannels, sizeof(uint8));

              const uint64 number_of_samples{ sound_resource._Samples[0].Size() };
              package_file->Write(&number_of_samples, sizeof(uint64));

              for (const DynamicArray<int16> &channel_samples : sound_resource._Samples)
              {
                package_file->Write(channel_samples.Data(), sizeof(int16) * channel_samples.Size());
              }

              //Update the current file size.
              current_file_size += sound_file_size;
            }
          }
        }

        if (number_of_files_read == 0)
        {
          break;
        }

        else
        {
          ++note_counter;

          if (note_counter >= 12)
          {
            ++octave_counter;

            note_counter = 0;
          }
        }
      }

      //Read the dead notes.
      for (;;)
      {
        //Remember the numer of files read.
        uint32 number_of_files_read{ 0 };

        //Read the notes.
        uint32 variation_counter{ 1 };

        //Calculate the type folder/string.
        const char* RESTRICT type_folder;
        const char* RESTRICT type_string;

        switch (type_index)
        {
        case 0:
        {
          type_folder = DI_FOLDER;
          type_string = "DI";

          break;
        }

        case 1:
        {
          type_folder = RHYTHM_FOLDER;
          type_string = "RHYTHM_GUITAR";

          break;
        }

        case 2:
        {
          type_folder = LEAD_FOLDER;
          type_string = "LEAD_GUITAR";

          break;
        }

        case 3:
        {
          type_folder = CLEAN_FOLDER;
          type_string = "CLEAN_GUITAR";

          break;
        }
        }

        for (;;)
        {
          sprintf(sound_file_buffer, "%s\\%s_%s_DEAD_NOTE_%u_%u.wav", type_folder, type_string, channel == Channel::LEFT ? "LEFT" : "RIGHT", dead_note_string_counter, variation_counter++);

          sound_resource._Samples.Clear();

          if (!WAVReader::Read(sound_file_buffer, &sound_resource))
          {
            break;
          }

          else
          {
            ++number_of_files_read;

            //Calculate the sound file size.
            uint64 sound_file_size{ 0 };

            sound_file_size += sizeof(uint8);
            sound_file_size += sizeof(uint64);
            sound_file_size += sizeof(float32);
            sound_file_size += sizeof(uint8);

            for (const DynamicArray<int16> &channel_samples : sound_resource._Samples)
            {
              sound_file_size += sizeof(int16) * channel_samples.Size();
            }

            //If the file size is too big, open the next package file.
            if (current_file_size + sound_file_size >= MAXIMUM_FILE_SIZE)
            {
              package_file->Close();
              delete package_file;

              char package_file_buffer[MAX_PATH];
              sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", OUTPUT_FOLDER, InfinityGuitarLogic::GetTypeString(static_cast<Type>(type_index + 1)), channel_string, ++package_counter);

              package_file = new BinaryFile<BinaryFileMode::OUT>(package_file_buffer);

              current_file_size = 0;
            }

            //Write the sound file to the file.
            package_file->Write(&sound_resource._SampleRate, sizeof(float32));
            package_file->Write(&sound_resource._NumberOfChannels, sizeof(uint8));

            const uint64 number_of_samples{ sound_resource._Samples[0].Size() };
            package_file->Write(&number_of_samples, sizeof(uint64));

            for (const DynamicArray<int16> &channel_samples : sound_resource._Samples)
            {
              package_file->Write(channel_samples.Data(), sizeof(int16) * channel_samples.Size());
            }

            //Update the current file size.
            current_file_size += sound_file_size;
          }
        }

        if (number_of_files_read == 0)
        {
          //Close the package file.
          package_file->Close();

          break;
        }

        else
        {
          ++dead_note_string_counter;
        }
      }
    }
  }

  BREAKPOINT();
}

/*
* Imports the packages.
*/
void InfinityGuitar::ImportPackages(const Channel channel, const Type type) NOEXCEPT
{
  //Can't load samples if none parameters are set.
  if (_CurrentChannel == Channel::NONE
    || _CurrentType == Type::NONE)
  {
    return;
  }

  //Signal that the plugin wants to reload notes.
  _WantsToReloadNotes.Set();

  //Wait for the plugin to become ready.
  _WantsToReloadNotes.Wait<WaitMode::YIELD>();

  //Cache the folder.
  DynamicString folder{ RetrievePluginPath("InfinityGuitar.vst3") };

#if USE_OUTPUT_LOG
  _OutputLog << "Folder: " << folder.Data() << std::endl;
#endif

  //Add the folder name.
  folder += "\\InfinityGuitarSamples";

  //Clear the notes.
  _Notes.Clear();

  //Open the first package file.
  uint32 package_counter{ 0 };
  BinaryFile<BinaryFileMode::IN> *RESTRICT package_file{ nullptr };

  {
    char package_file_buffer[MAX_PATH];
    sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", folder.Data(), InfinityGuitarLogic::GetTypeString(_CurrentType), channel == Channel::LEFT ? "LEFT" : "RIGHT", ++package_counter);

    package_file = new BinaryFile<BinaryFileMode::IN>(package_file_buffer);

    if (!(*package_file))
    {
#if USE_OUTPUT_LOG
      _OutputLog << "Tried to open file but failed: " << package_file_buffer << std::endl;
#endif

      //Signal that the plugin no longer wants to reload notes.
      _WantsToReloadNotes.Clear();

      return;
    }
  }

  //Keep track of if all notes have been read.
  bool all_notes_have_been_read{ false };
  bool all_dead_notes_has_been_read{ false };

  //Read all notes.
  while (!all_notes_have_been_read || !all_dead_notes_has_been_read)
  {
    //Has all the notes been read?
    if (_Notes.Size() < 61)
    {
      //Emplace a new note.
      _Notes.Emplace();
      InfinityGuitarNote& note{ _Notes.Back() };

      //Read the notes.
      for (uint8 i{ 0 }; i < 2; ++i)
      {
        for (uint8 j{ 0 }; j < 4; ++j)
        {
          //Read the sound resource.
          SoundResource *sound_resource;

          if (i == 0)
          {
            note._OpenNotes.Emplace();
            sound_resource = &note._OpenNotes.Back();
          }

          else
          {
            note._MutedNotes.Emplace();
            sound_resource = &note._MutedNotes.Back();
          }

          package_file->Read(&sound_resource->_SampleRate, sizeof(float32));
          package_file->Read(&sound_resource->_NumberOfChannels, sizeof(uint8));

          uint64 number_of_samples;
          package_file->Read(&number_of_samples, sizeof(uint64));

          sound_resource->_Samples.Upsize<true>(sound_resource->_NumberOfChannels);

          for (DynamicArray<int16>& channel_samples : sound_resource->_Samples)
          {
            channel_samples.Upsize<false>(number_of_samples);
            package_file->Read(channel_samples.Data(), sizeof(int16) * number_of_samples);
          }

          //Open a new package file, in case it has reached the end.
          if (package_file->HasReachedEndOfFile())
          {
            package_file->Close();
            delete package_file;

            char package_file_buffer[MAX_PATH];
            sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", folder.Data(), InfinityGuitarLogic::GetTypeString(_CurrentType), channel == Channel::LEFT ? "LEFT" : "RIGHT", ++package_counter);

            package_file = new BinaryFile<BinaryFileMode::IN>(package_file_buffer);

            if (!*package_file)
            {
              all_notes_have_been_read = true;

              break;
            }
          }
        }
      }
    }

    else
    {
      all_notes_have_been_read = true;

      for (uint8 string_index{ 0 }; string_index < 8; ++string_index)
      {
        for (uint8 variation_index{ 0 }; variation_index < 4; ++variation_index)
        {
          //Cache the sound resource.
          SoundResource *const RESTRICT sound_resource{ &_DeadNotes[string_index][variation_index] };

          //Read the sound resource.
          package_file->Read(&sound_resource->_SampleRate, sizeof(float32));
          package_file->Read(&sound_resource->_NumberOfChannels, sizeof(uint8));

          uint64 number_of_samples;
          package_file->Read(&number_of_samples, sizeof(uint64));

          sound_resource->_Samples.Upsize<true>(sound_resource->_NumberOfChannels);

          for (DynamicArray<int16>& channel_samples : sound_resource->_Samples)
          {
            channel_samples.Upsize<false>(number_of_samples);
            package_file->Read(channel_samples.Data(), sizeof(int16) * number_of_samples);
          }

          //Open a new package file, in case it has reached the end.
          if (package_file->HasReachedEndOfFile())
          {
            package_file->Close();
            delete package_file;

            char package_file_buffer[MAX_PATH];
            sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s_%u", folder.Data(), InfinityGuitarLogic::GetTypeString(_CurrentType), channel == Channel::LEFT ? "LEFT" : "RIGHT", ++package_counter);

            package_file = new BinaryFile<BinaryFileMode::IN>(package_file_buffer);
          }
        }

        all_dead_notes_has_been_read = true;
      }
    }
  }

  //Close the package file.
  package_file->Close();
  delete package_file;

  //Signal that the plugin no longer wants to reload notes.
  _WantsToReloadNotes.Clear();
}

/*
namespace Concurrency
{
  namespace CurrentThread
  {
    FORCE_INLINE void Yield() NOEXCEPT
    {
      //Just use the standard library.
      std::this_thread::yield();
    }
  }
}
*/