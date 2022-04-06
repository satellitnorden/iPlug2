//Header file.
#include "InfinityBass.h"

//Core.
#include <Core/General/DynamicString.h>

//File.
#include <File/Core/BinaryFile.h>
#include <File/Readers/WAVReader.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//Math.
#include <Math/Core/CatalystRandomMath.h>

//Windows.
#include <ShlObj_core.h>
#include <filesystem>

//Infinity Bass constants.
namespace InfinityBassConstants
{
  constexpr float32 MAXIMUM_HUMANIZE_GAIN{ 0.1f }; //0.025f step.
  constexpr float32 MAXIMUM_HUMANIZE_RELEASE{ 0.005f }; //0.025f step.
  constexpr float32 MAXIMUM_HUMANIZE_PLAYBACK_SPEED{ 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f };
  constexpr float32 MAXIMUM_HUMANIZE_OFFSET{ 4.0f };
  constexpr float32 MAXIMUM_HUMANIZE_STARTING_POSITION{ 0.00'25f };
  StaticArray<float32, 64> CONSTANT_POWER_LOOKUP;
}

/*
* Exports the regions needed. (:
*/
void ExportRegions() NOEXCEPT
{
  //Define constants.
  constexpr uint32 END_OCTAVE{ 4 };
  constexpr uint32 END_NOTE{ 1 };
  constexpr uint32 OPEN_NOTE_DURATION{ 4 };
  constexpr uint32 DEAD_NOTE_DURATION{ 1 };

  //Open the file.
  std::ofstream file{ "regions.csv" };

  //Insert header.
  file << "#,Name,Start,End,Length" << std::endl;

  //Add an empty first region.
  uint32 region_counter{ 1 };
  uint32 start_counter{ 1 };
  uint32 octave_counter{ 0 };
  uint32 note_counter{ 0 };

  file << "R" << ++region_counter << ",," << start_counter << ".1.00," << start_counter + 1 << ".1.00,1.0.00" << std::endl;

  ++start_counter;

  //Add all regions.
  while (!(octave_counter == END_OCTAVE && note_counter == END_NOTE))
  {
    //Add open notes.
    for (uint32 i{ 0 }; i < 4; ++i)
    {
      file << "R" << ++region_counter << "," << "INFINITY_BASS_OPEN_" << octave_counter << "_" << note_counter << "_" << i << "," <<  start_counter << ".1.00," << start_counter + OPEN_NOTE_DURATION << ".1.00,1.0.00" << std::endl;

      start_counter += OPEN_NOTE_DURATION;

      file << "R" << ++region_counter << ",," <<  start_counter << ".1.00," << start_counter + 1 << ".1.00,1.0.00" << std::endl;

      ++start_counter;
    }

    //Add dead notes.
    for (uint32 i{ 0 }; i < 4; ++i)
    {
      file << "R" << ++region_counter << "," << "INFINITY_BASS_DEAD_" << octave_counter << "_" << note_counter << "_" << i << "," <<  start_counter << ".1.00," << start_counter + DEAD_NOTE_DURATION << ".1.00,1.0.00" << std::endl;

      start_counter += DEAD_NOTE_DURATION;

      file << "R" << ++region_counter << ",," <<  start_counter << ".1.00," << start_counter + 1 << ".1.00,1.0.00" << std::endl;

      ++start_counter;
    }

    //Increment the note.
    ++note_counter;

    if (note_counter >= 12)
    {
      ++octave_counter;
      note_counter -= 12;
    }
  }

  //Inser the special semi colon. (:
  file << "semi;colon";

  file.close();
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

InfinityBass::InfinityBass(const iplug::InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Export regions.
  //ExportRegions();

  //Set up the constant power lookup.
  for (uint64 i{ 0 }, size{ InfinityBassConstants::CONSTANT_POWER_LOOKUP.Size() }; i < size; ++i)
  {
    InfinityBassConstants::CONSTANT_POWER_LOOKUP[i] = CatalystBaseMath::SquareRoot(static_cast<float32>(i + 1)) / static_cast<float32>(i + 1);
  }

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
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", INFINITYBASS_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachBackground(INFINITYBASS_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the info text.
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Open Notes: 100-127", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-96), "Muted Notes: 90-99", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));

    //Create the row of knobs.
    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      _HumanizeKnob = new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128), HUMANIZE_PARAM, "Humanize", style);

      pGraphics->AttachControl(_HumanizeKnob);
    }
  };
#endif

  //Export packages.
  //ExportPackages();

  //Import packages.
  ImportPackages();

#if USE_OUTPUT_LOG
  //Open the output log.
  char buffer[MAX_PATH];
  sprintf_s(buffer, "Infinity Bass Output Log %i.txt", CatalystRandomMath::RandomIntegerInRange<int32>(0, INT32_MAXIMUM));

  _OutputLog.open(buffer);
#endif
}

InfinityBass::~InfinityBass()
{
#if USE_OUTPUT_LOG
  //Close the output log.
  _OutputLog.close();
#endif
}

#if IPLUG_DSP
void InfinityBass::OnReset()
{
  for (PlayingNote& playing_note : _PlayingNotes)
  {
    playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
  }
}

void InfinityBass::OnParamChange(const int32 index)
{
  switch (index)
  {
    case HUMANIZE_PARAM:
    {
      _HumanizeFactor = static_cast<float32>(GetParam(HUMANIZE_PARAM)->Value() / 100.0);

      break;
    }
  }
}

void InfinityBass::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  //Nothing to process if there are no notes.
  if (_Notes.Empty())
  {
    return;
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
        const int32 velocity{ CatalystBaseMath::Maximum<int32>(message.mData2, 90) };

        //Determine the note index.
        const int32 note_index{ note_number - 28 };

        if (note_index >= 0 && note_index < _Notes.Size())
        {
          _PlayingNotes.Emplace();
          PlayingNote& playing_note{ _PlayingNotes.Back() };

          SoundResource* RESTRICT sound_resource;

          //Open notes.
          if (velocity >= 100)
          {
            sound_resource = &_Notes[note_index]._OpenNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * CatalystRandomMath::RandomFloatInRange(0.0f, InfinityBassConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor);
          }

          //Muted notes.
          else if (velocity >= 90)
          {
            sound_resource = &_Notes[note_index]._MutedNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * CatalystRandomMath::RandomFloatInRange(0.0f, InfinityBassConstants::MAXIMUM_HUMANIZE_STARTING_POSITION) * _HumanizeFactor);
          }

          playing_note._SoundResourcePlayer.SetSoundResource(ResourcePointer<SoundResource>(sound_resource));
          playing_note._SoundResourcePlayer.SetPlaybackSpeed(sound_resource->_SampleRate /  static_cast<float32>(GetSampleRate()) + CatalystRandomMath::RandomFloatInRange(-InfinityBassConstants::MAXIMUM_HUMANIZE_PLAYBACK_SPEED, InfinityBassConstants::MAXIMUM_HUMANIZE_PLAYBACK_SPEED) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.SetGain(1.0f + CatalystRandomMath::RandomFloatInRange(-InfinityBassConstants::MAXIMUM_HUMANIZE_GAIN, InfinityBassConstants::MAXIMUM_HUMANIZE_GAIN) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues( 0.001f,
                                                                              0.01f,
                                                                              1.0f,
                                                                              0.01f + CatalystRandomMath::RandomFloatInRange(0.0f, InfinityBassConstants::MAXIMUM_HUMANIZE_RELEASE) * _HumanizeFactor);
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
      const float32 constant_power_gain{ InfinityBassConstants::CONSTANT_POWER_LOOKUP[number_of_playing_notes - 1] };

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

      const float32 constant_power_gain_1{ InfinityBassConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes), InfinityBassConstants::CONSTANT_POWER_LOOKUP.Size() - 1)] };
      const float32 constant_power_gain_2{ InfinityBassConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes) + 1, InfinityBassConstants::CONSTANT_POWER_LOOKUP.Size() - 1)] };

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

void InfinityBass::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  iplug::IMidiMsg offset_message{ msg };
  //offset_message.mOffset += CatalystBaseMath::Round<int32>(CatalystRandomMath::RandomFloatInRange(-InfinityBassConstants::MAXIMUM_HUMANIZE_OFFSET, InfinityBassConstants::MAXIMUM_HUMANIZE_OFFSET) * _HumanizeFactor);

  _MidiQueue.Add(offset_message);
}
#endif

/*
* Exports the packages.
*/
void InfinityBass::ExportPackages() NOEXCEPT
{
  //Define constants.
  constexpr char* OUTPUT_FOLDER{ "C:\\Github\\Audio-Data\\Plugins\\Instruments\\InfinityBassSamples" };
  constexpr char* INPUT_FOLDER{ "C:\\Infinity Bass Temporary" };
  constexpr uint64 MAXIMUM_FILE_SIZE{ 100'000'000 };
  {
    //Open the first package file.
    uint32 package_counter{ 0 };
    BinaryFile<IOMode::Out> *RESTRICT package_file{ nullptr };

    {
      char package_file_buffer[MAX_PATH];
      sprintf_s(package_file_buffer, "%s\\INFINITY_BASS_PACKAGE_%u", OUTPUT_FOLDER, ++package_counter);

      package_file = new BinaryFile<IOMode::Out>(package_file_buffer);
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
        //Read the notes.
        uint32 variation_counter{ 0 };

        for (;;)
        {
          sprintf(sound_file_buffer, "%s\\INFINITY_BASS_%s_%u_%u_%u.wav", INPUT_FOLDER, i == 0 ? "OPEN" : "DEAD", octave_counter, note_counter, variation_counter++);

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
              sprintf_s(package_file_buffer, "%s\\INFINITY_BASS_PACKAGE_%u", OUTPUT_FOLDER, ++package_counter);

              package_file = new BinaryFile<IOMode::Out>(package_file_buffer);

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
  }
}

/*
* Imports the packages.
*/
void InfinityBass::ImportPackages() NOEXCEPT
{
  //Cache the folder.
  DynamicString folder{ RetrievePluginPath("InfinityBass.vst3") };

#if USE_OUTPUT_LOG
  _OutputLog << "Folder: " << folder.Data() << std::endl;
#endif

  //Add the folder name.
  folder += "\\InfinityBassSamples";

  //Clear the notes.
  _Notes.Clear();

  //Open the first package file.
  uint32 package_counter{ 0 };
  BinaryFile<IOMode::In> *RESTRICT package_file{ nullptr };

  {
    char package_file_buffer[MAX_PATH];
    sprintf_s(package_file_buffer, "%s\\INFINITY_BASS_PACKAGE_%u", folder.Data(), ++package_counter);

    package_file = new BinaryFile<IOMode::In>(package_file_buffer);

    if (!(*package_file))
    {
#if USE_OUTPUT_LOG
      _OutputLog << "Tried to open file but failed: " << package_file_buffer << std::endl;
#endif
      return;
    }
  }

  //Keep track of if all notes have been read.
  bool all_notes_have_been_read{ false };

  //Read all notes.
  while (!all_notes_have_been_read)
  {
    //Emplace a new note.
    _Notes.Emplace();
    InfinityBassNote& note{ _Notes.Back() };

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
          sprintf_s(package_file_buffer, "%s\\INFINITY_BASS_PACKAGE_%u", folder.Data(), ++package_counter);

          package_file = new BinaryFile<IOMode::In>(package_file_buffer);

          if (!*package_file)
          {
            all_notes_have_been_read = true;

            break;
          }
        }
      }
    }
  }

  //Close the package file.
  package_file->Close();
  delete package_file;
}

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