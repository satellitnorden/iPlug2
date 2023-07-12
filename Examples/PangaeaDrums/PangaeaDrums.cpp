//Header file.
#include "PangaeaDrums.h"

//Core.
#include <Core/General/DynamicString.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//File.
#undef IN
#undef OUT
#define CATALYST_PLATFORM_WINDOWS
#include <File/Core/BinaryFile.h>
#include <File/Readers/WAVReader.h>

//Math.
#include <Math/Core/CatalystRandomMath.h>

//Windows.
#include <filesystem>

static_assert(sizeof(unsigned int) == (32 / 8), "Size of uint32 is not what it should be!");
static_assert(sizeof(int) == (32 / 8), "Size of int32 is not what it should be!");

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
 * Exports the regions needed. (:
 */
void ExportRegions() NOEXCEPT
{
  //Define constants.
  const unsigned int NUMBER_OF_DRUM_PARTS{ 24 };
  constexpr StaticArray<const char *RESTRICT, NUMBER_OF_DRUM_PARTS> DRUM_PART_NAMES
  {
    "KICK",
    "SNARE",
    "TOM_1",
    "TOM_2",
    "TOM_3",
    "HIHAT_PEDAL",
    "HIHAT_FULLY_CLOSED",
    "HIHAT_LEVEL_1",
    "HIHAT_FULLY_OPEN",
    "CHINA",
    "LEFT_CRASH",
    "RIGHT_CRASH",
    "LEFT_WIDE_CRASH",
    "SPLASH",
    "RIDE_BOW",
    "RIDE_BELL",
    "RIDE_EDGE",
    "TOM_4",
    "STACK",
    "HIHAT_LEVEL_0",
    "HIHAT_LEVEL_2",
    "HIHAT_LEVEL_3",
    "HIHAT_LEVEL_4",
    "SNARE_SIDE_STICK"
  };
  constexpr StaticArray<int, NUMBER_OF_DRUM_PARTS> DRUM_PART_DURATIONS
  {
    1,
    2,
    2,
    2,
    2,
    1,
    1,
    2,
    4,
    4,
    4,
    4,
    4,
    4,
    4,
    4,
    4,
    2,
    4,
    1,
    2,
    4,
    4,
    1
  };

  // Open the file.
  std::ofstream file{"regions.csv"};

  // Insert header.
  file << "#,Name,Start,End,Length" << std::endl;

  // Add an empty first region.
  unsigned int region_counter{ 1 };
  unsigned int start_counter{ 1 };

  file << "R" << ++region_counter << ",," << start_counter << ".1.00," << start_counter + 1 << ".1.00,1.0.00" << std::endl;

  ++start_counter;

  // Add all regions.
  for (uint64 i{ 0 }; i < DRUM_PART_NAMES.Size(); ++i)
  {
    const char* const RESTRICT drum_part_name{DRUM_PART_NAMES[i]};
    const unsigned int drum_part_duration{ static_cast<unsigned int>(DRUM_PART_DURATIONS[i]) };

    for (unsigned int velocity_index{ 0 }; velocity_index < AlesisDrumsConstants::NUMBER_OF_LAYERS; ++velocity_index)
    {
      for (unsigned int sample_index{ 0 }; sample_index < AlesisDrumsConstants::NUMBER_OF_LAYERS; ++sample_index)
      {
        file << "R" << ++region_counter << "," << drum_part_name << "_VELOCITY_" << velocity_index << "_" << sample_index + 1 << "," << start_counter << ".1.00," << start_counter + drum_part_duration
             << ".1.00,1.0.00" << std::endl;

        start_counter += drum_part_duration;

        file << "R" << ++region_counter << ",," << start_counter << ".1.00," << start_counter + 1 << ".1.00,1.0.00" << std::endl;

        ++start_counter;
      }
    }
  }

  // Inser the special semi colon. (:
  file << "semi;colon";

  file.close();

  // Breakpoint. \o/
  BREAKPOINT();
}

/*
 *	Rounds a number down to the nearest integer.
 */
template <typename TYPE>
FORCE_INLINE constexpr static NO_DISCARD TYPE Floor(const float32 number) NOEXCEPT
{
  return number >= 0.0f ? static_cast<TYPE>(static_cast<int>(number)) : static_cast<TYPE>(static_cast<int>(number - 1.0f));
}

PangaeaDrums::PangaeaDrums(const InstanceInfo& info)
: Plugin(info, MakeConfig(0, 1))
{
  //Define constants.
  constexpr float32 CYMBALS_PANNING{ 0.0f };

  //ExportRegions();

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Load the font.
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::IColor(255, 25, 25, 25));

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{pGraphics->GetBounds()};

    //Create the header text.
    {
      _Text = new iplug::igraphics::ITextControl(bounds.GetCentredInside(512.0f).GetVShifted(-128.0f), "Alesis Drums", iplug::igraphics::IText(64));

      pGraphics->AttachControl(_Text);
    }
  };
#endif

  //Initialize the drum part properties.
  _DrumPartProperties[UNDERLYING(DrumPart::KICK)]._DrumPartString = "KICK";
  _DrumPartProperties[UNDERLYING(DrumPart::KICK)]._MidiNumber = 24;
  _DrumPartProperties[UNDERLYING(DrumPart::KICK)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::KICK)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::KICK)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::SNARE)]._DrumPartString = "SNARE";
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE)]._MidiNumber = 26;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::SNARE_SIDE_STICK)]._DrumPartString = "SNARE_SIDE_STICK";
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE_SIDE_STICK)]._MidiNumber = 27;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE_SIDE_STICK)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE_SIDE_STICK)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::SNARE_SIDE_STICK)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::TOM_1)]._DrumPartString = "TOM_1";
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_1)]._MidiNumber = 35;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_1)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_1)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_1)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::TOM_2)]._DrumPartString = "TOM_2";
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_2)]._MidiNumber = 36;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_2)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_2)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_2)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::TOM_3)]._DrumPartString = "TOM_3";
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_3)]._MidiNumber = 37;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_3)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_3)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_3)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::TOM_4)]._DrumPartString = "TOM_4";
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_4)]._MidiNumber = 38;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_4)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_4)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::TOM_4)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::CHINA)]._DrumPartString = "CHINA";
  _DrumPartProperties[UNDERLYING(DrumPart::CHINA)]._MidiNumber = 67;
  _DrumPartProperties[UNDERLYING(DrumPart::CHINA)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::CHINA)]._Pan = CYMBALS_PANNING / 4.0f * 4.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::CHINA)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BOW)]._DrumPartString = "RIDE_BOW";
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BOW)]._MidiNumber = 62;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BOW)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BOW)]._Pan = CYMBALS_PANNING / 4.0f * 2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BOW)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BELL)]._DrumPartString = "RIDE_BELL";
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BELL)]._MidiNumber = 63;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BELL)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BELL)]._Pan = CYMBALS_PANNING / 4.0f * 2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_BELL)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_EDGE)]._DrumPartString = "RIDE_EDGE";
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_EDGE)]._MidiNumber = 64;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_EDGE)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_EDGE)]._Pan = CYMBALS_PANNING / 4.0f * 2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::RIDE_EDGE)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_CRASH)]._DrumPartString = "LEFT_CRASH";
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_CRASH)]._MidiNumber = 52;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_CRASH)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_CRASH)]._Pan = CYMBALS_PANNING / 4.0f * -1.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_CRASH)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::SPLASH)]._DrumPartString = "SPLASH";
  _DrumPartProperties[UNDERLYING(DrumPart::SPLASH)]._MidiNumber = 73;
  _DrumPartProperties[UNDERLYING(DrumPart::SPLASH)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::SPLASH)]._Pan = 0.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::SPLASH)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::RIGHT_CRASH)]._DrumPartString = "RIGHT_CRASH";
  _DrumPartProperties[UNDERLYING(DrumPart::RIGHT_CRASH)]._MidiNumber = 54;
  _DrumPartProperties[UNDERLYING(DrumPart::RIGHT_CRASH)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::RIGHT_CRASH)]._Pan = CYMBALS_PANNING / 4.0f * 1.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::RIGHT_CRASH)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_PEDAL)]._DrumPartString = "HIHAT_PEDAL";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_PEDAL)]._MidiNumber = 48;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_PEDAL)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_PEDAL)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_PEDAL)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_CLOSED)]._DrumPartString = "HIHAT_FULLY_CLOSED";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_CLOSED)]._MidiNumber = 40;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_CLOSED)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_CLOSED)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_CLOSED)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_0)]._DrumPartString = "HIHAT_LEVEL_0";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_0)]._MidiNumber = 42;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_0)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_0)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_0)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_1)]._DrumPartString = "HIHAT_LEVEL_1";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_1)]._MidiNumber = 43;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_1)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_1)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_1)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_2)]._DrumPartString = "HIHAT_LEVEL_2";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_2)]._MidiNumber = 44;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_2)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_2)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_2)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_3)]._DrumPartString = "HIHAT_LEVEL_3";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_3)]._MidiNumber = 45;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_3)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_3)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_3)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_4)]._DrumPartString = "HIHAT_LEVEL_4";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_4)]._MidiNumber = 46;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_4)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_4)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_LEVEL_4)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_OPEN)]._DrumPartString = "HIHAT_FULLY_OPEN";
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_OPEN)]._MidiNumber = 47;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_OPEN)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_OPEN)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::HIHAT_FULLY_OPEN)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_WIDE_CRASH)]._DrumPartString = "LEFT_WIDE_CRASH";
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_WIDE_CRASH)]._MidiNumber = 56;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_WIDE_CRASH)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_WIDE_CRASH)]._Pan = CYMBALS_PANNING / 4.0f * -4.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::LEFT_WIDE_CRASH)]._Gain = 1.0f;

  _DrumPartProperties[UNDERLYING(DrumPart::STACK)]._DrumPartString = "STACK";
  _DrumPartProperties[UNDERLYING(DrumPart::STACK)]._MidiNumber = 78;
  _DrumPartProperties[UNDERLYING(DrumPart::STACK)]._VelocityCurve = DrumPartProperties::VelocityCurve::LINEAR;
  _DrumPartProperties[UNDERLYING(DrumPart::STACK)]._Pan = CYMBALS_PANNING / 4.0f * -2.0f;
  _DrumPartProperties[UNDERLYING(DrumPart::STACK)]._Gain = 1.0f;

  //Export packages.
  //ExportPackages();

  //Import packages.
  ImportPackages();
}

#if IPLUG_DSP
void PangaeaDrums::OnReset()
{
  for (PlayingNote& playing_note : _PlayingNotes)
  {
    playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(static_cast<float32>(GetSampleRate()));
  }
}

void PangaeaDrums::ProcessBlock(sample** inputs, sample** outputs, const int number_of_frames)
{
  //Calculate the time per sample.
  const float32 time_per_sample{ 1.0f / static_cast<float32>(GetSampleRate()) };

  const int number_of_channels{NOutChansConnected()};

  for (int i{ 0 }; i < number_of_frames; ++i)
  {
    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg& message{_MidiQueue.Peek()};

      if (message.mOffset > i)
      {
        break;
      }

      if (message.StatusMsg() == iplug::IMidiMsg::kNoteOn)
      {
        // Determine which drum part to play.
        unsigned int triggered_drum_parts_counter{0};
        StaticArray<DrumPart, 2> triggered_drum_parts{ DrumPart::NONE, DrumPart::NONE };

        for (uint64 j{0}; j < UNDERLYING(DrumPart::NUMBER_OF_DRUM_PARTS); ++j)
        {
          if (message.mData1 == _DrumPartProperties[j]._MidiNumber)
          {
            triggered_drum_parts[triggered_drum_parts_counter++] = static_cast<DrumPart>(j);
          }
        }

        for (const DrumPart triggered_drum_part : triggered_drum_parts)
        {
          if (triggered_drum_part != DrumPart::NONE)
          {
            //Extract the velocity.
            float32 velocity{ static_cast<float32>(message.mData2) / 127.0f };

            switch (_DrumPartProperties[UNDERLYING(triggered_drum_part)]._VelocityCurve)
            {
              case DrumPartProperties::VelocityCurve::LINEAR:
              {
                //Nothing to do here.

                break;
              }

              case DrumPartProperties::VelocityCurve::HALF_INVERSE_SQUARE:
              {
                velocity = CatalystBaseMath::LinearlyInterpolate(velocity, CatalystBaseMath::InverseSquare(velocity), 0.5f);

                break;
              }

              case DrumPartProperties::VelocityCurve::INVERSE_SQUARE:
              {
                velocity = CatalystBaseMath::InverseSquare(velocity);

                break;
              }

              case DrumPartProperties::VelocityCurve::MAXIMUM:
              {
                velocity = 1.0f;

                break;
              }
            }

            // Calculate the velocity indices and gain.
            uint64 first_velocity_index;
            uint64 second_velocity_index;
            float32 first_gain;
            float32 second_gain;

            if (_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources.Size() > 1)
            {
              first_velocity_index = Floor<uint64>(velocity * static_cast<float32>(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources.LastIndex()));
              second_velocity_index = CatalystBaseMath::Minimum<uint64>(first_velocity_index + 1, _DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources.LastIndex());

              {
                const float32 pan{ CatalystBaseMath::Fractional(velocity * static_cast<float32>(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources.LastIndex())) * 2.0f - 1.0f };

                constexpr float32 SQUARE_ROOT_OF_TWO_OVER_TWO{ 0.70710678118f };

                const float32 angle{ CatalystBaseMath::DegreesToRadians(45.0f) * pan };
                const float32 angle_sine{ CatalystBaseMath::Sine(angle) };
                const float32 angle_cosine{ CatalystBaseMath::Cosine(angle) };

                first_gain = SQUARE_ROOT_OF_TWO_OVER_TWO * (angle_cosine - angle_sine);
                second_gain = SQUARE_ROOT_OF_TWO_OVER_TWO * (angle_cosine + angle_sine);
              }
            }

            else
            {
              first_velocity_index = 0;
              first_gain = velocity;
              second_gain = velocity;
            }

            first_gain *= _DrumPartProperties[UNDERLYING(triggered_drum_part)]._Gain;
            second_gain *= _DrumPartProperties[UNDERLYING(triggered_drum_part)]._Gain;

            // Add the playing note(s).
            if (first_gain > 0.0f)
            {
              _PlayingNotes.Emplace();
              PlayingNote& playing_note{_PlayingNotes.Back()};

              const uint64 index{ _DrumPartProperties[UNDERLYING(triggered_drum_part)]._RandomIndexer.Next() };

              playing_note._SoundResourcePlayer.SetSoundResource(ResourcePointer<SoundResource>(&_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources[first_velocity_index][index]));
              playing_note._SoundResourcePlayer.SetPlaybackSpeed(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources[first_velocity_index][index]._SampleRate / static_cast<float32>(GetSampleRate()));
              playing_note._SoundResourcePlayer.SetGain(first_gain);
              playing_note._SoundResourcePlayer.SetPan(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._Pan);
              playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
              playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues(0.0f, 0.0f, 1.0f, 0.1f);
              playing_note._SoundResourcePlayer.GetADSREnvelope().EnterAttackStage();
              playing_note._DrumPart = triggered_drum_part;
            }

            if (second_gain > 0.0f)
            {
              _PlayingNotes.Emplace();
              PlayingNote& playing_note{_PlayingNotes.Back()};

              const uint64 index{ _DrumPartProperties[UNDERLYING(triggered_drum_part)]._RandomIndexer.Next( )};

              playing_note._SoundResourcePlayer.SetSoundResource(ResourcePointer<SoundResource>(&_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources[second_velocity_index][index]));
              playing_note._SoundResourcePlayer.SetPlaybackSpeed(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._SoundResources[second_velocity_index][index]._SampleRate / static_cast<float32>(GetSampleRate()));
              playing_note._SoundResourcePlayer.SetGain(second_gain);
              playing_note._SoundResourcePlayer.SetPan(_DrumPartProperties[UNDERLYING(triggered_drum_part)]._Pan);
              playing_note._SoundResourcePlayer.GetADSREnvelope().SetSampleRate(GetSampleRate());
              playing_note._SoundResourcePlayer.GetADSREnvelope().SetStageValues(0.0f, 0.0f, 1.0f, 0.1f);
              playing_note._SoundResourcePlayer.GetADSREnvelope().EnterAttackStage();
              playing_note._DrumPart = triggered_drum_part;
            }
          }

          // If the triggered drum part is a high hat pedal, stop all open hihat sounds.
          if (triggered_drum_part == DrumPart::HIHAT_PEDAL)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_FULLY_CLOSED || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_0 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_1
                  || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_2 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_3 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4
                  || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_FULLY_CLOSED)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_LEVEL_0 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_1 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_2
                  || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_3 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4 || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_LEVEL_0)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_LEVEL_1 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_2 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_3
                  || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4 || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_LEVEL_1)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_LEVEL_2 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_3 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4
                  || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_LEVEL_2)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_LEVEL_3 || playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4 || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_LEVEL_3)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_LEVEL_4 || playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }

          if (triggered_drum_part == DrumPart::HIHAT_LEVEL_4)
          {
            for (PlayingNote& playing_note : _PlayingNotes)
            {
              if (playing_note._DrumPart == DrumPart::HIHAT_FULLY_OPEN)
              {
                playing_note._SoundResourcePlayer.Stop();
              }
            }
          }
        }
      }

      else if (message.StatusMsg() == iplug::IMidiMsg::kPolyAftertouch)
      {
        // Determine which drum part to play.
        DrumPart triggered_drum_part{ DrumPart::NONE };

        for (uint64 j{ 0 }; j < UNDERLYING(DrumPart::NUMBER_OF_DRUM_PARTS); ++j)
        {
          if (message.mData1 == _DrumPartProperties[j]._MidiNumber)
          {
            triggered_drum_part = static_cast<DrumPart>(j);

            break;
          }
        }

        // Stop all playing notes for this drum part.
        for (PlayingNote &playing_note : _PlayingNotes)
        {
          if (playing_note._DrumPart == triggered_drum_part)
          {
            playing_note._SoundResourcePlayer.Stop();
          }
        }
      }

      _MidiQueue.Remove();
    }

    // Check if any playing notes should be removed.
    bool note_was_removed{ false };

    do
    {
      note_was_removed = false;

      for (PlayingNote &playing_note : _PlayingNotes)
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

    for (int channel_index{ 0 }; channel_index < 2; ++channel_index)
    {
      float32 value{ 0.0f };

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        value += playing_note._SoundResourcePlayer.NextSample(channel_index);
      }

      outputs[channel_index][i] = value;
    }

    for (PlayingNote& playing_note : _PlayingNotes)
    {
      playing_note._SoundResourcePlayer.Advance();
    }
  }
}

void PangaeaDrums::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}

/*
* Exports packages.
*/
void PangaeaDrums::ExportPackages() NOEXCEPT
{
  //Define constants.
  constexpr uint64 MAXIMUM_FILE_SIZE{ 1'000'000'000 };

  //Open the first package file.
  unsigned int package_counter{0};
  BinaryFile<BinaryFileMode::OUT> *RESTRICT package_file{ nullptr };

  {
    char package_file_buffer[128];
    sprintf_s(package_file_buffer, "C:\\Users\\Denni\\Google Drive\\Share Folder\\Plugins\\PangaeaDrumsPackages\\PangaeaDrumsPackage_%u", ++package_counter);

    package_file = new BinaryFile<BinaryFileMode::OUT>(package_file_buffer);
  }

  // Export all packages.
  char sound_file_buffer[128];
  uint64 current_file_size{0};

  for (unsigned int i{0}; i < UNDERLYING(DrumPart::NUMBER_OF_DRUM_PARTS); ++i)
  {
    DrumPartProperties& drum_part_properties{_DrumPartProperties[i]};

    bool found_velocity{ true };
    unsigned int velocity_index{ 0 };

    while (found_velocity)
    {
      for (unsigned int j{ 0 }; j < AlesisDrumsConstants::NUMBER_OF_LAYERS; ++j)
      {
        SoundResource sound_resource;

        sprintf_s(sound_file_buffer, "C:\\Infinity Drums Temporary\\%s_VELOCITY_%u_%u.wav", drum_part_properties._DrumPartString, velocity_index, j + 1);

        if (!WAVReader::Read(sound_file_buffer, &sound_resource))
        {
          found_velocity = false;

          break;
        }

        // Calculate the sound file size.
        uint64 sound_file_size{ 0 };

        sound_file_size += sizeof(uint8);
        sound_file_size += sizeof(uint64);
        sound_file_size += sizeof(float32);
        sound_file_size += sizeof(uint8);

        for (const DynamicArray<int16> &channel_samples : sound_resource._Samples)
        {
          sound_file_size += sizeof(int16) * channel_samples.Size();
        }

        // If the file size is too big, open the next package file.
        if (current_file_size + sound_file_size >= MAXIMUM_FILE_SIZE)
        {
          package_file->Close();
          delete package_file;

          char package_file_buffer[128];
          sprintf_s(package_file_buffer, "C:\\Users\\Denni\\Google Drive\\Share Folder\\Plugins\\PangaeaDrumsPackages\\PangaeaDrumsPackage_%u", ++package_counter);

          package_file = new BinaryFile<BinaryFileMode::OUT>(package_file_buffer);

          current_file_size = 0;
        }

        // Write the sound file to the file.
        package_file->Write(&sound_resource._SampleRate, sizeof(float32));
        package_file->Write(&sound_resource._NumberOfChannels, sizeof(uint8));

        const uint64 number_of_samples{sound_resource._Samples[0].Size()};
        package_file->Write(&number_of_samples, sizeof(uint64));

        for (const DynamicArray<int16>& channel_samples : sound_resource._Samples)
        {
          package_file->Write(channel_samples.Data(), sizeof(int16) * channel_samples.Size());
        }

        // Update the current file size.
        current_file_size += sound_file_size;
      }

      ++velocity_index;
    }
  }

  // Close the package file.
  package_file->Close();

  BREAKPOINT();
}

/*
* Imports packages.
*/
void PangaeaDrums::ImportPackages() NOEXCEPT
{
  // Cache the folder.
  DynamicString folder{RetrievePluginPath("PangaeaDrums.vst3")};

  // Add the folder name.
  folder += "\\PangaeaDrumsPackages";

  // Open the first package file.
  unsigned int package_counter{ 0 };
  BinaryFile<BinaryFileMode::IN>* RESTRICT package_file{nullptr};

  {
    char package_file_buffer[128];
    sprintf_s(package_file_buffer, "%s\\PangaeaDrumsPackage_%u", folder.Data(), ++package_counter);

    package_file = new BinaryFile<BinaryFileMode::IN>(package_file_buffer);
  }

  // Read all drum part properties.
  for (unsigned int i{ 0 }; i < UNDERLYING(DrumPart::NUMBER_OF_DRUM_PARTS); ++i)
  {
    // Cache the drum part properties.
    DrumPartProperties &drum_part_properties{_DrumPartProperties[i]};

    // Reserve the appropriate amount of space for the velocity layers.
    drum_part_properties._SoundResources.Upsize<true>(AlesisDrumsConstants::NUMBER_OF_LAYERS);

    // Read all the sound resources.
    for (StaticArray<SoundResource, AlesisDrumsConstants::NUMBER_OF_LAYERS>& sub_sound_resources : drum_part_properties._SoundResources)
    {
      for (SoundResource& sound_resource : sub_sound_resources)
      {
        package_file->Read(&sound_resource._SampleRate, sizeof(float32));
        package_file->Read(&sound_resource._NumberOfChannels, sizeof(uint8));

        uint64 number_of_samples;
        package_file->Read(&number_of_samples, sizeof(uint64));

        sound_resource._Samples.Upsize<true>(sound_resource._NumberOfChannels);

        for (DynamicArray<int16>& channel_samples : sound_resource._Samples)
        {
          channel_samples.Upsize<false>(number_of_samples);
          package_file->Read(channel_samples.Data(), sizeof(int16) * number_of_samples);
        }

        if (package_file->HasReachedEndOfFile())
        {
          package_file->Close();
          delete package_file;

          char package_file_buffer[128];
          sprintf_s(package_file_buffer, "%s\\PangaeaDrumsPackage_%u", folder.Data(), ++package_counter);

          package_file = new BinaryFile<BinaryFileMode::IN>(package_file_buffer);
        }
      }
    }
  }
}
#endif
