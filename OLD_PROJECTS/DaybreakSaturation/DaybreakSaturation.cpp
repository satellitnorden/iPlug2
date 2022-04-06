//Header file.
#include "DaybreakSaturation.h"

//File.
#include <File/Core/FileCore.h>
#include <File/Readers/WAVReader.h>

//Sound.
#include <Sound/SoundMixComponent.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

DaybreakSaturation::DaybreakSaturation(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  GetParam(VARIATION)->InitInt("VARIATION", 1, 1, 3);
  GetParam(BOOST)->InitDouble("BOOST", 1.0, 0.0, 8.0, 0.01);

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
    pGraphics->LoadFont("Default-VST-Font", DAYBREAKSATURATION_FN);

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_DARK_GRAY);
    
    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the header text.
    {
      _Text = new iplug::igraphics::ITextControl(bounds.GetCentredInside(300).GetVShifted(-200), "Daybreak Saturation", iplug::igraphics::IText(32));

      pGraphics->AttachControl(_Text);
    }

    //Create upper row of knobs.
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(-125), VARIATION));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(125), BOOST));
  };
#endif
}

#if IPLUG_DSP
void DaybreakSaturation::OnReset()
{
  
}

void DaybreakSaturation::OnParamChange(const int32 index)
{
  switch (index)
  {
    case VARIATION:
    {
      _Variation = static_cast<uint32>(GetParam(VARIATION)->Value());

      break;
    }

    case BOOST:
    {
      _Boost = static_cast<float32>(GetParam(BOOST)->Value());

      break;
    }
  }
}

void DaybreakSaturation::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  const int32 number_of_channels{ NOutChansConnected() };

  for (int32 sample_index{ 0 }; sample_index < number_of_frames; ++sample_index)
  {
    for (int32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      float32 sample{ inputs ? static_cast<float32>(inputs[channel_index][sample_index]) : 0.0f };

      SaturationSoundMixComponent::State state;

      state._Variation = static_cast<SaturationSoundMixComponent::Variation>(_Variation - 1);
      state._Boost = _Boost;

      SaturationSoundMixComponent::Process(&state, &sample);

      outputs[channel_index][sample_index] = sample;
    }
  }
}

void DaybreakSaturation::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  
}
#endif