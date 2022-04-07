//Header file.
#include "InfinityGuitar.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//Core.
#include <Core/Algorithms/SortingAlgorithms.h>
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

//Humanization.
#include "../../The-Infinity-Construct/Code/Include/Main/DjenteratorCore.h"

//Infinity guitar.
#include "InfinityGuitarHumanization.h"

//Constants.
#define USE_TRACK_KNOB (1)

// InfinityGuitar constants.
namespace InfinityGuitarConstants
{
  constexpr float32 MAXIMUM_HUMANIZE_RELEASE{ 0.005f }; // 0.025f step.
  StaticArray<float32, 64> CONSTANT_POWER_LOOKUP;
}

// Infinity guitar logic.
namespace InfinityGuitarLogic
{

  /*
  * Returns the string for a certain track.
  */
  FORCE_INLINE RESTRICTED NO_DISCARD const char *const RESTRICT GetTrackString(const InfinityGuitar::Track track) NOEXCEPT
  {
    switch (track)
    {
      case InfinityGuitar::Track::LEFT:
      {
        return "LEFT";
      }

      case InfinityGuitar::Track::RIGHT:
      {
        return "RIGHT";
      }

      default:
      {
        ASSERT(false, "Invalid case!");

        return "";
      }
    }
  }

  /*
  * Returns the string for a certain channel.
  */
  FORCE_INLINE RESTRICTED NO_DISCARD const char* const RESTRICT GetChannelString(const InfinityGuitar::Channel channel) NOEXCEPT
  {
    switch (channel)
    {
      case InfinityGuitar::Channel::DI:
      {
        return "DI";
      }

      case InfinityGuitar::Channel::RHYTHM:
      {
        return "RHYTHM";
      }

      case InfinityGuitar::Channel::LEAD:
      {
        return "LEAD";
      }

      case InfinityGuitar::Channel::CLEAN:
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

InfinityGuitar::InfinityGuitar(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Set up the constant power lookup.
  for (uint64 i{ 0 }, size{InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size()}; i < size; ++i)
  {
    InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[i] = CatalystBaseMath::SquareRoot(static_cast<float32>(i + 1)) / static_cast<float32>(i + 1);
  }

  GetParam(TRACK_PARAM)->InitInt("TRACK", 0, 0, 1);
  GetParam(CHANNEL_PARAM)->InitInt("CHANNEL", 0, 0, 3);
  GetParam(HUMANIZE_PARAM)->InitDouble("HUMANIZE", 100.0, 0.0, 200.0, 0.01);
  GetParam(RELEASE_PARAM)->InitDouble("RELEASE", 0.01, 0.01, 16.0, 0.01);
    
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Load the font.
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", INFINITYGUITAR_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachBackground(INFINITYGUITAR_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{pGraphics->GetBounds()};

    //CHECK THIS LATER
    //pGraphics->EnableMouseOver(true);
    //pGraphics->EnableMultiTouch(true);

    //Create the info text.
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Open Notes: 100-127", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-96), "Muted Notes: 90-99", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-64), "Tapped Open Notes: 80-89", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-32), "Tapped Muted notes : 70-79", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(0), "Dead notes : 60-69", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));

    //Create the row of knobs.
    {
#if USE_TRACK_KNOB
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      iplug::igraphics::IVKnobControl *const RESTRICT track_knob{ new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(-192), TRACK_PARAM, "Track", style) };

      pGraphics->AttachControl(track_knob);
#else
      pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(75).GetHShifted(-128), "Track", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));

      iplug::igraphics::IVButtonControl *const RESTRICT di_button{ new iplug::igraphics::IVButtonControl(bounds.GetCentredInside(48).GetVShifted(110).GetHShifted(-128), [this](iplug::igraphics::IControl* control) { GetParam(TRACK_PARAM)->Set(static_cast<double>(Track::LEFT)); }, "LEFT") };

      pGraphics->AttachControl(di_button);
#endif
      GetParam(TRACK_PARAM)->SetDisplayFunc([this](const double, WDL_String& string)
      {
        string.Set(InfinityGuitarLogic::GetTrackString(_WantedTrack));
      });
    }

    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      iplug::igraphics::IVKnobControl* const RESTRICT channel_knob{ new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(-64), CHANNEL_PARAM, "Channel", style) };

      pGraphics->AttachControl(channel_knob);

      GetParam(CHANNEL_PARAM)->SetDisplayFunc([this](const double, WDL_String& string)
      {
        string.Set(InfinityGuitarLogic::GetChannelString(_WantedChannel));
      });
    }

    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      iplug::igraphics::IVKnobControl *const RESTRICT humanize_knob{ new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(64), HUMANIZE_PARAM, "Humanize", style) };

      pGraphics->AttachControl(humanize_knob);

      GetParam(HUMANIZE_PARAM)->SetDisplayFunc([this](const double value, WDL_String& string)
      {
        char buffer[64];
        sprintf_s(buffer, "%i%%", static_cast<int>(value));

        string.Set(buffer);
      });
    }

    {
      iplug::igraphics::IVStyle style;

      style.labelText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
      style.valueText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

      iplug::igraphics::IVKnobControl *const RESTRICT release_knob{ new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(128).GetVShifted(128).GetHShifted(192), RELEASE_PARAM, "Release", style)};

      pGraphics->AttachControl(release_knob);

      GetParam(RELEASE_PARAM)->SetDisplayFunc([this](const double value, WDL_String &string)
      {
        char buffer[64];
        sprintf_s(buffer, "%.1f", value);

        string.Set(buffer);
      });
    }

    {
      iplug::igraphics::ITextControl *const RESTRICT information_text{ new iplug::igraphics::ITextControl( bounds.GetCentredInside(512).GetVShifted(235), "INFORMATION TEXT", iplug::igraphics::IText(20, iplug::igraphics::IColor(255, 255, 255, 255))) };

      information_text->SetAnimation([this](IControl *const RESTRICT control)
      {
        if (_Error != Error::NONE)
        {
          switch (_Error)
          {
            case Error::COULDNT_LOAD_SAMPLES:
            {
              static_cast<ITextControl*>(control)->SetStr("Couldn't load samples!");

              break;
            }

            default:
            {
              ASSERT(false, "Invalid case!");

              break;
            }
          }
        }

        else
        {
          switch (_CurrentLoadingState)
          {
            case LoadingState::IDLE:
            {
              static_cast<ITextControl*>(control)->SetStr("");

              break;
            }

            case LoadingState::LOADING:
            {
              static_cast<ITextControl*>(control)->SetStr("Loading samples...");

              break;
            }

            default:
            {
              ASSERT(false, "Invalid case!");

              break;
            }
          }
        }
        
      });

      pGraphics->AttachControl(information_text);
    }
  };
#endif

  //Export packages.
  //ExportPackages();

#if USE_OUTPUT_LOG
  //Open the output log.
  char buffer[MAX_PATH];
  sprintf_s(buffer, "Infinity Guitar Output Log %i.txt", CatalystRandomMath::RandomIntegerInRange<int32>(0, INT32_MAXIMUM));

  _OutputLog.open(buffer);
#endif
}

InfinityGuitar::~InfinityGuitar()
{
  //Wait for the async loading thread, if necessary.
  if (_AsyncLoadingThread)
  {
    _AsyncLoadingThread->join();
    delete _AsyncLoadingThread;
    _AsyncLoadingThread = nullptr;
  }

#if USE_OUTPUT_LOG
  // Close the output log.
  _OutputLog.close();
#endif
}

#if IPLUG_DSP
void InfinityGuitar::OnReset()
{
  for (PlayingNote &playing_note : _PlayingNotes)
  {
    playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
  }
}

void InfinityGuitar::OnParamChange(int index)
{
  switch (index)
  {
    case TRACK_PARAM:
    {
      const Track new_track{ static_cast<Track>(GetParam(TRACK_PARAM)->Int() + 1) };

      if (_WantedTrack != new_track)
      {
        _WantedTrack = new_track;
      }

    #if USE_OUTPUT_LOG
        //_OutputLog << "OnParamChange - track - " << static_cast<int32>(new_track) << std::endl;
    #endif

      break;
    }

    case CHANNEL_PARAM:
    {
      const Channel new_Channel{static_cast<Channel>(GetParam(CHANNEL_PARAM)->Int() + 1)};

      if (_WantedChannel != new_Channel)
      {
        _WantedChannel = new_Channel;
      }

    #if USE_OUTPUT_LOG
        //_OutputLog << "OnParamChange - channel - " << static_cast<int32>(new_channel) << std::endl;
    #endif

      break;
    }

    case HUMANIZE_PARAM:
    {
      _HumanizeFactor = static_cast<float>(GetParam(HUMANIZE_PARAM)->Value() / 100.0);

      break;
    }

    case RELEASE_PARAM:
    {
      _Release = static_cast<float>(GetParam(RELEASE_PARAM)->Value());

      break;
    }
  }
}

void InfinityGuitar::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  //Check loading of samples.
  switch (_CurrentLoadingState)
  {
    case LoadingState::IDLE:
    {
      //Should new samples be loaded?
      if (_CurrentTrack != _WantedTrack || _CurrentChannel != _WantedChannel)
      {
        //Remove all playing notes.
        _PlayingNotes.Clear();

        //Async import the new packages!
        AsyncImportPackages();

        //Update the current loading state.
        _CurrentLoadingState = LoadingState::LOADING;

        return;
      }

      break;
    }

    case LoadingState::LOADING:
    {
      //Has the loading finished?
      if (_HasFinishedLoadingSamples.IsSet())
      {
        //Join the thread.
        _AsyncLoadingThread->join();

        //Destroy the async loading thread.
        delete _AsyncLoadingThread;
        _AsyncLoadingThread = nullptr;

        //Set the current track and channel.
        _CurrentTrack = _LoadedTrack;
        _CurrentChannel = _LoadedChannel;

        //Update the current loading state.
        _CurrentLoadingState = LoadingState::IDLE;
      }

      else
      {
        return;
      }

      break;
    }

    default:
    {
      ASSERT(false, "Invalid case!");

      break;
    }
  }

  //Nothing to process if there are no notes.
  if (_CurrentTrack == Track::NONE
      || _CurrentChannel == Channel::NONE
      || _Notes.Empty())
  {
    return;
  }

  //Cache the number of channels.
  const int number_of_channels{ NOutChansConnected() };

  //Set up the humanization.
  InfinityGuitarHumanization humanization;

  switch (_CurrentChannel)
  {
    case Channel::DI:
    {
      humanization._GainVariation = DjenteratorHumanization::DIGuitar::GAIN_VARIATION * _HumanizeFactor;
      humanization._PlaybackRateVariation = DjenteratorHumanization::DIGuitar::PLAYBACK_RATE_VARIATION * _HumanizeFactor;

      break;
    }

    case Channel::RHYTHM:
    {
      humanization._GainVariation = DjenteratorHumanization::RhythmGuitar::GAIN_VARIATION * _HumanizeFactor;
      humanization._PlaybackRateVariation = DjenteratorHumanization::RhythmGuitar::PLAYBACK_RATE_VARIATION * _HumanizeFactor;

      break;
    }

    case Channel::LEAD:
    {
      humanization._GainVariation = DjenteratorHumanization::LeadGuitar::GAIN_VARIATION * _HumanizeFactor;
      humanization._PlaybackRateVariation = DjenteratorHumanization::LeadGuitar::PLAYBACK_RATE_VARIATION * _HumanizeFactor;

      break;
    }

    case Channel::CLEAN:
    {
      humanization._GainVariation = DjenteratorHumanization::CleanGuitar::GAIN_VARIATION * _HumanizeFactor;
      humanization._PlaybackRateVariation = DjenteratorHumanization::CleanGuitar::PLAYBACK_RATE_VARIATION * _HumanizeFactor;

      break;
    }

    default:
    {
      ASSERT(false, "Invalid case!");

      break;
    }
  }

  for (int i{ 0 }; i < number_of_frames; ++i)
  {
    //Remember if notes were modified.
    bool modified_notes{ false };

    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg& message{_MidiQueue.Peek()};

      if (message.mOffset > i)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{message.StatusMsg()};

      if (status == iplug::IMidiMsg::kNoteOn)
      {
        //Extract the note number and velocity.
        const int note_number{ message.mData1 };
        const int velocity{ CatalystBaseMath::Maximum<int>(message.mData2, 60) };

        //Determine the note index.
        const int note_index{ note_number - 40 };

        if (note_index >= 0 && note_index < _Notes.Size())
        {
          _PlayingNotes.Emplace();
          PlayingNote &playing_note{ _PlayingNotes.Back() };

          SoundResource *RESTRICT sound_resource;

          //Open notes.
          if (velocity >= 100)
          {
            sound_resource = &_Notes[note_index]._OpenNotes[_RandomIndexer.Next()];
          }

          //Muted notes.
          else if (velocity >= 90)
          {
            sound_resource = &_Notes[note_index]._MutedNotes[_RandomIndexer.Next()];
          }

          /*
           * Tapped open notes/hammer on open notes/hammer off open notes.
           * This is faked by just playing the open notes, but offset the starting sample position a bit.
           */
          else if (velocity >= 80)
          {
            float32 offset;

            switch (_CurrentChannel)
            {
              case Channel::DI:
              {
                offset = DjenteratorHumanization::DIGuitar::TAPPED_OPEN_NOTES_START_OFFSET;

                break;
              }

              case Channel::RHYTHM:
              {
                offset = DjenteratorHumanization::RhythmGuitar::TAPPED_OPEN_NOTES_START_OFFSET;

                break;
              }

              case Channel::LEAD:
              {
                offset = DjenteratorHumanization::LeadGuitar::TAPPED_OPEN_NOTES_START_OFFSET;

                break;
              }

              case Channel::CLEAN:
              {
                offset = DjenteratorHumanization::CleanGuitar::TAPPED_OPEN_NOTES_START_OFFSET;

                break;
              }

              default:
              {
                ASSERT(false, "Invalid case!");

                break;
              }
            }

            sound_resource = &_Notes[note_index]._OpenNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * offset);
          }

          /*
           * Tapped muted notes/hammer on muted notes/hammer off muted notes.
           * This is faked by just playing the muted notes, but offset the starting sample position a bit.
           */
          else if (velocity >= 70)
          {
            float32 offset;

            switch (_CurrentChannel)
            {
              case Channel::DI:
              {
                offset = DjenteratorHumanization::DIGuitar::TAPPED_MUTED_NOTES_START_OFFSET;

                break;
              }

              case Channel::RHYTHM:
              {
                offset = DjenteratorHumanization::RhythmGuitar::TAPPED_MUTED_NOTES_START_OFFSET;

                break;
              }

              case Channel::LEAD:
              {
                offset = DjenteratorHumanization::LeadGuitar::TAPPED_MUTED_NOTES_START_OFFSET;

                break;
              }

              case Channel::CLEAN:
              {
                offset = DjenteratorHumanization::CleanGuitar::TAPPED_MUTED_NOTES_START_OFFSET;

                break;
              }

              default:
              {
                ASSERT(false, "Invalid case!");

                break;
              }
            }

            sound_resource = &_Notes[note_index]._MutedNotes[_RandomIndexer.Next()];
            playing_note._SoundResourcePlayer.SetCurrentSample(GetSampleRate() * offset);
          }

          /*
           * Dead notes.
           */
          else if (velocity >= 60)
          {
            sound_resource = &_DeadNotes[CatalystBaseMath::Minimum<int>(note_index / 7, 7)][_DeadNoteRandomIndexer.Next()];
          }

          playing_note._SoundResourcePlayer.SetSoundResource(ResourcePointer<SoundResource>(sound_resource));
          playing_note._SoundResourcePlayer.SetPlaybackSpeed(sound_resource->_SampleRate / static_cast<float32>(GetSampleRate()) + CatalystRandomMath::RandomFloatInRange(-humanization._PlaybackRateVariation, humanization._PlaybackRateVariation));
          playing_note._SoundResourcePlayer.SetGain(1.0f + CatalystRandomMath::RandomFloatInRange(-humanization._GainVariation, humanization._GainVariation));
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
          playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues(0.001f, 0.01f, 1.0f, _Release + CatalystRandomMath::RandomFloatInRange(0.0f, InfinityGuitarConstants::MAXIMUM_HUMANIZE_RELEASE) * _HumanizeFactor);
          playing_note._SoundResourcePlayer.GetADSREnvelope().EnterAttackStage();
          playing_note._NoteNumber = message.mData1;
        }

        //Remember that notes were modified.
        modified_notes = true;
      }

      else if (status == iplug::IMidiMsg::kNoteOff)
      {
        for (PlayingNote &playing_note : _PlayingNotes)
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

          //Remember that notes were modified.
          modified_notes = true;

          break;
        }
      }
    } while (note_was_removed);

    //Apply constant power gain.
    {
      float32 number_of_playing_notes{ 0.0f };

      for (const PlayingNote& playing_note : _PlayingNotes)
      {
        number_of_playing_notes += playing_note._SoundResourcePlayer.GetADSREnvelope().CalculateCurrentMultiplier();
      }

      const float32 constant_power_gain_1{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes), InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size() - 1)]};
      const float32 constant_power_gain_2{ InfinityGuitarConstants::CONSTANT_POWER_LOOKUP[CatalystBaseMath::Minimum<uint64>(static_cast<uint64>(number_of_playing_notes) + 1, InfinityGuitarConstants::CONSTANT_POWER_LOOKUP.Size() - 1)]};

      const float32 alpha{CatalystBaseMath::Fractional(number_of_playing_notes)};

      for (PlayingNote &playing_note : _PlayingNotes)
      {
        playing_note._SoundResourcePlayer.SetGain(CatalystBaseMath::LinearlyInterpolate(constant_power_gain_1, constant_power_gain_2, alpha));
      }
    }

    //Sort notes, if necessary.
    if (modified_notes && !_PlayingNotes.Empty())
    {
      SortingAlgorithms::InsertionSort<PlayingNote>
      (
        _PlayingNotes.Begin(),
        _PlayingNotes.End(),
        nullptr,
        [](const void *const RESTRICT, const PlayingNote *const RESTRICT first, const PlayingNote *const RESTRICT second)
        {
          return first->_NoteNumber < second->_NoteNumber;
        }
      );
    }

    //Apply chord gain.
    if (_PlayingNotes.Size() > 1
      && _HumanizeFactor > 0.0f)
    {
      for (uint64 i{ 0 }; i < _PlayingNotes.Size(); ++i)
      {
        const float current_gain{ _PlayingNotes[i]._SoundResourcePlayer.GetGain() };
        const float alpha{ static_cast<float>(i) / static_cast<float>(_PlayingNotes.Size() - 1) };

        _PlayingNotes[i]._SoundResourcePlayer.SetGain(current_gain + (CatalystBaseMath::LinearlyInterpolate(DjenteratorHumanization::CHORD_GAIN, -DjenteratorHumanization::CHORD_GAIN, alpha) * _HumanizeFactor * _PlayingNotes[i]._SoundResourcePlayer.GetADSREnvelope().CalculateCurrentMultiplier()));
      }
    }

    for (int j{ 0 }; j < number_of_channels; ++j)
    {
      float value{ 0.0f };

      for (PlayingNote &playing_note : _PlayingNotes)
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

void InfinityGuitar::ProcessMidiMsg(const IMidiMsg& msg)
{
  iplug::IMidiMsg offset_message{ msg };
  //offset_message.mOffset += CatalystBaseMath::Round<int32>(CatalystRandomMath::RandomFloatInRange(-InfinityGuitarConstants::MAXIMUM_HUMANIZE_OFFSET, InfinityGuitarConstants::MAXIMUM_HUMANIZE_OFFSET) * _HumanizeFactor);

  _MidiQueue.Add(offset_message);
}

/*
 * Exports the packages.
 */
void InfinityGuitar::ExportPackages() NOEXCEPT
{
  // Define constants.
  constexpr char *OUTPUT_FOLDER{ "C:\\Users\\Denni\\Google Drive\\Share Folder\\Plugins\\InfinityGuitarPackages" };
  constexpr char *DI_FOLDER{ "C:\\Users\\Denni\\Google Drive\\Share Folder\\Plugins\\InfinityGuitarSamples" };
  constexpr char *RHYTHM_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Djent\\Sounds\\Guitar\\Rhythm" };
  constexpr char *LEAD_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Sounds\\Guitar\\Lead" };
  constexpr char *CLEAN_FOLDER{ "C:\\Github\\The-Infinity-Construct\\Resources\\Raw\\Djent\\Sounds\\Guitar\\Clean" };

  // Iterate over all channels.
  for (uint8 channel_index{ 0 }; channel_index < 4; ++channel_index)
  {
    // Iterate over both tracks.
    for (uint8 track_index{ 0 }; track_index < 2; ++track_index)
    {
      // Cache the track.
      const Track track{static_cast<Track>(track_index + 1)};

      // Cache the track string.
      const char *RESTRICT track_string{ track == Track::LEFT ? "LEFT" : "RIGHT" };

      // Open the first package file.
      char package_file_buffer[MAX_PATH];
      sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s", OUTPUT_FOLDER, InfinityGuitarLogic::GetChannelString(static_cast<Channel>(channel_index + 1)), track_string);

      BinaryFile<BinaryFileMode::OUT> package_file{ package_file_buffer };

      // Export all packages.
      char sound_file_buffer[MAX_PATH];
      unsigned octave_counter{ 0 };
      unsigned note_counter{ 0 };
      SoundResource sound_resource;
      unsigned dead_note_string_counter{ 0 };

      //Read the regular notes.
      for (;;)
      {
        //Remember the numer of files read.
        unsigned number_of_files_read{ 0 };

        //Read the different types of notes.
        for (uint8 i{ 0 }; i < 2; ++i)
        {
          // Cache the note string.
          const char *RESTRICT note_string{ i == 0 ? "OPEN" : "MUTED" };

          // Read the notes.
          unsigned variation_counter{ 1 };

          // Calculate the channel folder/string.
          const char *RESTRICT channel_folder;
          const char *RESTRICT channel_string;

          switch (channel_index)
          {
            case 0:
            {
              channel_folder = DI_FOLDER;
              channel_string = "DI";

              break;
            }

            case 1:
            {
              channel_folder = RHYTHM_FOLDER;
              channel_string = "RHYTHM_GUITAR";

              break;
            }

            case 2:
            {
              channel_folder = LEAD_FOLDER;
              channel_string = "LEAD_GUITAR";

              break;
            }

            case 3:
            {
              channel_folder = CLEAN_FOLDER;
              channel_string = "CLEAN_GUITAR";

              break;
            }
          }

          for (;;)
          {
            sprintf(sound_file_buffer, "%s\\%s_%s_%u_%u_%s_%u.wav", channel_folder, channel_string, track == Track::LEFT ? "LEFT" : "RIGHT", octave_counter, note_counter, note_string, variation_counter++);

            sound_resource._Samples.Clear();

            if (!WAVReader::Read(sound_file_buffer, &sound_resource))
            {
              break;
            }

            else
            {
              ++number_of_files_read;

              // Write the sound file to the file.
              package_file.Write(&sound_resource._SampleRate, sizeof(float32));
              package_file.Write(&sound_resource._NumberOfChannels, sizeof(uint8));

              const uint64 number_of_samples{sound_resource._Samples[0].Size()};
              package_file.Write(&number_of_samples, sizeof(uint64));

              for (const DynamicArray<int16>& channel_samples : sound_resource._Samples)
              {
                package_file.Write(channel_samples.Data(), sizeof(int16) * channel_samples.Size());
              }
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

      // Read the dead notes.
      for (;;)
      {
        // Remember the numer of files read.
        unsigned number_of_files_read{ 0 };

        // Read the notes.
        unsigned variation_counter{ 1 };

        // Calculate the channel folder/string.
        const char* RESTRICT channel_folder;
        const char* RESTRICT channel_string;

        switch (channel_index)
        {
          case 0:
          {
            channel_folder = DI_FOLDER;
            channel_string = "DI";

            break;
          }

          case 1:
          {
            channel_folder = RHYTHM_FOLDER;
            channel_string = "RHYTHM_GUITAR";

            break;
          }

          case 2:
          {
            channel_folder = LEAD_FOLDER;
            channel_string = "LEAD_GUITAR";

            break;
          }

          case 3:
          {
            channel_folder = CLEAN_FOLDER;
            channel_string = "CLEAN_GUITAR";

            break;
          }
        }

        for (;;)
        {
          sprintf(sound_file_buffer, "%s\\%s_%s_DEAD_NOTE_%u_%u.wav", channel_folder, channel_string, track == Track::LEFT ? "LEFT" : "RIGHT", dead_note_string_counter, variation_counter++);

          sound_resource._Samples.Clear();

          if (!WAVReader::Read(sound_file_buffer, &sound_resource))
          {
            break;
          }

          else
          {
            ++number_of_files_read;

            // Write the sound file to the file.
            package_file.Write(&sound_resource._SampleRate, sizeof(float32));
            package_file.Write(&sound_resource._NumberOfChannels, sizeof(uint8));

            const uint64 number_of_samples{sound_resource._Samples[0].Size()};
            package_file.Write(&number_of_samples, sizeof(uint64));

            for (const DynamicArray<int16>& channel_samples : sound_resource._Samples)
            {
              package_file.Write(channel_samples.Data(), sizeof(int16) * channel_samples.Size());
            }
          }
        }

        if (number_of_files_read == 0)
        {
          // Close the package file.
          package_file.Close();

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
* Async imports packages.
*/
void InfinityGuitar::AsyncImportPackages() NOEXCEPT
{
  //Set the loaded track and channel.
  _LoadedTrack = _WantedTrack;
  _LoadedChannel = _WantedChannel;

  //Spawn the thread.
  _AsyncLoadingThread = new std::thread([this]()
  {
    ImportPackages(_LoadedTrack, _LoadedChannel);
  });
}

/*
 * Imports the packages.
 */
void InfinityGuitar::ImportPackages(const Track track, const Channel channel) NOEXCEPT
{
  //Cache the folder.
  DynamicString folder{ RetrievePluginPath("InfinityGuitar.vst3") };

#if USE_OUTPUT_LOG
  _OutputLog << "Folder: " << folder.Data() << std::endl;
#endif

  // Add the folder name.
  folder += "\\InfinityGuitarPackages";

  //Clear the notes.
  _Notes.Clear();

  //Open the first package file.
  char package_file_buffer[MAX_PATH];
  sprintf_s(package_file_buffer, "%s\\INFINITY_GUITAR_PACKAGE_%s_%s", folder.Data(), InfinityGuitarLogic::GetChannelString(channel), track == Track::LEFT ? "LEFT" : "RIGHT");

  BinaryFile<BinaryFileMode::IN> package_file{ package_file_buffer };

  if (!package_file)
  {
#if USE_OUTPUT_LOG
    _OutputLog << "Tried to open file but failed: " << package_file_buffer << std::endl;
#endif

    //Set the error.
    _Error = Error::COULDNT_LOAD_SAMPLES;

    //Signal that the plugin is "finished" loading samples, I guess.
    _HasFinishedLoadingSamples.Set();

    return;
  }

  // Keep track of if all notes have been read.
  bool all_notes_have_been_read{false};
  bool all_dead_notes_has_been_read{false};

  // Read all notes.
  while (!all_notes_have_been_read || !all_dead_notes_has_been_read)
  {
    // Has all the notes been read?
    if (_Notes.Size() < 61)
    {
      // Emplace a new note.
      _Notes.Emplace();
      InfinityGuitarNote& note{_Notes.Back()};

      // Read the notes.
      for (uint8 i{ 0 }; i < 2; ++i)
      {
        for (uint8 j{ 0 }; j < 4; ++j)
        {
          // Read the sound resource.
          SoundResource* sound_resource;

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

          package_file.Read(&sound_resource->_SampleRate, sizeof(float32));
          package_file.Read(&sound_resource->_NumberOfChannels, sizeof(uint8));

          uint64 number_of_samples;
          package_file.Read(&number_of_samples, sizeof(uint64));

          sound_resource->_Samples.Upsize<true>(sound_resource->_NumberOfChannels);

          for (DynamicArray<int16>& channel_samples : sound_resource->_Samples)
          {
            channel_samples.Upsize<false>(number_of_samples);
            package_file.Read(channel_samples.Data(), sizeof(int16) * number_of_samples);
          }

          // Open a new package file, in case it has reached the end.
          if (package_file.HasReachedEndOfFile())
          {
            all_notes_have_been_read = true;

            break;
          }
        }
      }
    }

    else
    {
      all_notes_have_been_read = true;

      for (uint8 string_index{0}; string_index < 8; ++string_index)
      {
        for (uint8 variation_index{0}; variation_index < 4; ++variation_index)
        {
          // Cache the sound resource.
          SoundResource* const RESTRICT sound_resource{&_DeadNotes[string_index][variation_index]};

          // Read the sound resource.
          package_file.Read(&sound_resource->_SampleRate, sizeof(float32));
          package_file.Read(&sound_resource->_NumberOfChannels, sizeof(uint8));

          uint64 number_of_samples;
          package_file.Read(&number_of_samples, sizeof(uint64));

          sound_resource->_Samples.Upsize<true>(sound_resource->_NumberOfChannels);

          for (DynamicArray<int16>& channel_samples : sound_resource->_Samples)
          {
            channel_samples.Upsize<false>(number_of_samples);
            package_file.Read(channel_samples.Data(), sizeof(int16) * number_of_samples);
          }
        }

        all_dead_notes_has_been_read = true;
      }
    }
  }

  // Close the package file.
  package_file.Close();

  //Signal the the plugin is finished loading samples.
  _HasFinishedLoadingSamples.Set();
}
#endif
