//Header file.
#include "InfinitySampler.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"

//File.
#include <File/Core/FileCore.h>
#include <File/Readers/WAVReader.h>

//Math.
#include <Math/Core/CatalystRandomMath.h>

//Windows.
#include <filesystem>

//InfinitySampler constants.
namespace InfinitySamplerConstants
{
  constexpr float MAXIMUM_HUMANIZE_GAIN{ 0.1f }; //0.025f step.
}

InfinitySampler::InfinitySampler(const iplug::InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  GetParam(MIDI_NOTE)->InitInt("MIDI NOTE", 24, 0, 127);
  GetParam(MAX_SAMPLES)->InitInt("SAMPLES", 16, 1, 16);
  GetParam(VELOCITY_CURVE)->InitInt("CURVE", 0, 0, 2);
  GetParam(HUMANIZE)->InitDouble("HUMANIZE", 0.0, 0.0, 100.0, 0.01);
  GetParam(ATTACK_VALUE)->InitDouble("ATTACK", 0.001, 0.001, 10.0, 0.001);
  GetParam(DECAY_VALUE)->InitDouble("DECAY", 0.01, 0.01, 10.0, 0.01);
  GetParam(SUSTAIN_VALUE)->InitDouble("SUSTAIN", 1.0, 0.01, 10.0, 0.01);
  GetParam(RELEASE_VALUE)->InitDouble("RELEASE", 0.01, 0.01, 10.0, 0.01);

#if IPLUG_EDITOR
  //Set the make graphics function.
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.0f);
  };

  //Set the layout function.
  mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics)
  {
    //Load the font.
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_DARK_GRAY);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the header text.
    {
      _Text = new iplug::igraphics::ITextControl(bounds.GetCentredInside(400).GetVShifted(-200), "Infinity Sampler", iplug::igraphics::IText(64));

      pGraphics->AttachControl(_Text);
    }

    //Create the file button.
    {
      iplug::igraphics::IVButtonControl* const file_button{ new iplug::igraphics::IVButtonControl(bounds.GetCentredInside(100).GetVShifted(-100).GetHShifted(-100), [this](iplug::igraphics::IControl* control) { SelectFile(); }, "FILE") };

      pGraphics->AttachControl(file_button);
    }

    //Create the folder button.
    {
      iplug::igraphics::IVButtonControl* const folder_button{ new iplug::igraphics::IVButtonControl(bounds.GetCentredInside(100).GetVShifted(-100).GetHShifted(100), [this](iplug::igraphics::IControl* control) { SelectFolder(); }, "FOLDER") };

      pGraphics->AttachControl(folder_button);
    }

    //Create upper row of knobs.
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(-150), MIDI_NOTE));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(-50), MAX_SAMPLES));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(50), VELOCITY_CURVE));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(150), HUMANIZE));

    //Create lower row of knobs.
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(150).GetHShifted(-150), ATTACK_VALUE));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(150).GetHShifted(-50), DECAY_VALUE));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(150).GetHShifted(50), SUSTAIN_VALUE));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(150).GetHShifted(150), RELEASE_VALUE));
  };
#endif
}

#if IPLUG_DSP
void InfinitySampler::OnReset()
{
  for (PlayingNote& playing_note : _PlayingNotes)
  {
    playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
  }
}

void InfinitySampler::OnParamChange(const int32 index)
{
  switch (index)
  {
  case MIDI_NOTE:
  {
    break;
  }

  case HUMANIZE:
  {
    _HumanizeFactor = static_cast<float>(GetParam(HUMANIZE)->Value() / 100.0);

    break;
  }
  }
}

void InfinitySampler::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  //Nothing to process if there are no sounds.
  if (_SoundResources.Empty())
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

      if (message.mData1 == GetParam(MIDI_NOTE)->Int())
      {
        const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

        if (status == iplug::IMidiMsg::kNoteOn)
        {
          //Extract the note number and velocity.
          const int32 velocity{ message.mData2 };
          float gain{ static_cast<float>(velocity) / 127.0f };

          switch (static_cast<int>(GetParam(VELOCITY_CURVE)->Int()))
          {
          case 0:
          {


            break;
          }

          case 1:
          {
            gain = gain * gain;

            break;
          }

          case 2:
          {
            gain = 1.0f - ((1.0f - gain) * (1.0f - gain));

            break;
          }
          }

          //Determine the note index.
          _PlayingNotes.Emplace();
          PlayingNote& playing_note{ _PlayingNotes.Back() };

          if (_SoundResources.Size() > 1 && GetParam(MAX_SAMPLES)->Int() > 1)
          {
            const uint64 maximum{ CatalystBaseMath::Minimum<uint64>(_SoundResources.Size(), GetParam(MAX_SAMPLES)->Int()) - 1 };

            _LastPlayedNoteIndex = SelectNextIndex(maximum, _LastPlayedNoteIndex);
          }

          else
          {
            _LastPlayedNoteIndex = 0;
          }

          playing_note._SoundResourcePlayer.SetSoundResource(&_SoundResources[_LastPlayedNoteIndex]);
          playing_note._SoundResourcePlayer.SetPlaybackSpeed(_SoundResources[_LastPlayedNoteIndex]._SampleRate /  static_cast<float32>(GetSampleRate()));
          playing_note._SoundResourcePlayer.SetGain(gain + CatalystRandomMath::RandomFloatInRange(-InfinitySamplerConstants::MAXIMUM_HUMANIZE_GAIN, InfinitySamplerConstants::MAXIMUM_HUMANIZE_GAIN) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues(GetParam(ATTACK_VALUE)->Value(),
            GetParam(DECAY_VALUE)->Value(),
            GetParam(SUSTAIN_VALUE)->Value(),
            GetParam(RELEASE_VALUE)->Value());
          playing_note._SoundResourcePlayer.GetADSREnvelope().EnterAttackStage();
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

void InfinitySampler::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}
#endif

bool InfinitySampler::SerializeState(iplug::IByteChunk& chunk) const
{
  const bool parent_succeeded{ iplug::Plugin::SerializeState(chunk) };

  chunk.PutBytes(&_SaveState, sizeof(InfinitySamplerSaveState));

  return parent_succeeded;
}

int InfinitySampler::UnserializeState(const iplug::IByteChunk& chunk, int startPos)
{
  const int parent_pos{ iplug::Plugin::UnserializeState(chunk, startPos) };

  chunk.GetBytes(&_SaveState, sizeof(InfinitySamplerSaveState), parent_pos);

  switch (_SaveState._SaveState)
  {
    case InfinitySamplerSaveState::SaveState::FILE:
    {
      ReadSoundResource(_SaveState._FilePath);

      break;
    }

    case InfinitySamplerSaveState::SaveState::FOLDER:
    {
      ReadSoundResources(_SaveState._FolderPath);

      break;
    }
  }

  return parent_pos + sizeof(InfinitySamplerSaveState);
}

/*
* Selects the file.
*/
void InfinitySampler::SelectFile() NOEXCEPT
{
  //Browse for the folder.
  DynamicString chosen_file;

  if (File::BrowseForFile(false, &chosen_file))
  {
    _SaveState._SaveState = InfinitySamplerSaveState::SaveState::FILE;

    //Copy the chosen folder to the save state.
    Memory::Copy(_SaveState._FilePath, chosen_file.Data(), sizeof(char) * MAX_PATH);

    //Read the sound resource.
    ReadSoundResource(_SaveState._FilePath);
  }
}

/*
* Selects the folder.
*/
void InfinitySampler::SelectFolder() NOEXCEPT
{
  //Browse for the folder.
  DynamicString chosen_folder;

  if (File::BrowseForFolder(&chosen_folder))
  {
    _SaveState._SaveState = InfinitySamplerSaveState::SaveState::FOLDER;

    //Copy the chosen folder to the save state.
    Memory::Copy(_SaveState._FolderPath, chosen_folder.Data(), sizeof(char) * MAX_PATH);

    //Read the sound resources.
    ReadSoundResources(_SaveState._FolderPath);
  }
}

/*
* Reads the sound resource.
*/
void InfinitySampler::ReadSoundResource(const char *const RESTRICT file) NOEXCEPT
{
  _SoundResources.Clear();
  _SoundResources.Reserve(1);

  _SoundResources.Emplace();

  SoundResource &sound_resource{ _SoundResources.Back() };

  WAVReader::Read(file, &sound_resource);
}

/*
* Reads all the sound resources.
*/
void InfinitySampler::ReadSoundResources(const char *const RESTRICT folder) NOEXCEPT
{
  _SoundResources.Clear();

  //Count the number of resources in the folder.
  uint64 number_of_resources{ 0 };

  for (const auto& resource : std::filesystem::directory_iterator(std::string(folder)))
  {
    ++number_of_resources;
  }

  _SoundResources.Reserve(number_of_resources);

  //Iterate over all files in the folder in the folder and read the sound resources.
  for (const auto& resource_in_folder : std::filesystem::directory_iterator(std::string(folder)))
  {
    _SoundResources.Emplace();

    SoundResource &sound_resource{ _SoundResources.Back() };

    WAVReader::Read(resource_in_folder.path().string().c_str(), &sound_resource);
  }
}

/*
* Selects the next index to play.
*/
uint64 InfinitySampler::SelectNextIndex(const uint64 maximum, const uint64 last) NOEXCEPT
{
  uint64 next{ CatalystRandomMath::RandomIntegerInRange<uint64>(0, maximum - 1) };

  while (next == last && maximum > 1)
  {
    next = CatalystRandomMath::RandomIntegerInRange<uint64>(0, maximum - 1);
  }

  return next;
}