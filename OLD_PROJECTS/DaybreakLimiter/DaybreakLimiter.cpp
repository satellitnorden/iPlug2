//Header file.
#include "DaybreakLimiter.h"

//File.
#include <File/Core/FileCore.h>
#include <File/Readers/WAVReader.h>

//Sound.
#include <Sound/SoundMixComponent.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

DaybreakLimiter::DaybreakLimiter(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  GetParam(BOOST)->InitDouble("BOOST", 1.0, 0.0, 4.0, 0.01);
  GetParam(CEILING)->InitDouble("CEILING", 1.0, 0.0, 1.0, 0.01);

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
    pGraphics->LoadFont("Default-VST-Font", DAYBREAKLIMITER_FN);

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_DARK_GRAY);
    
    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the header text.
    {
      _Text = new iplug::igraphics::ITextControl(bounds.GetCentredInside(300).GetVShifted(-200), "Daybreak Limiter", iplug::igraphics::IText(32));

      pGraphics->AttachControl(_Text);
    }

    //Create upper row of knobs.
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(-125), BOOST));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(100).GetVShifted(25).GetHShifted(125), CEILING));
  };
#endif
}

#if IPLUG_DSP
void DaybreakLimiter::OnParamChange(const int32 index)
{
  switch (index)
  {
    case BOOST:
    {
      _Boost = static_cast<float32>(GetParam(BOOST)->Value());

      break;
    }

    case CEILING:
    {
      _Ceiling = static_cast<float32>(GetParam(CEILING)->Value());

      break;
    }
  }
}

void DaybreakLimiter::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  const int32 number_of_channels{ NOutChansConnected() };

  for (int32 sample_index{ 0 }; sample_index < number_of_frames; ++sample_index)
  {
    for (int32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      float32 sample{ inputs ? static_cast<float32>(inputs[channel_index][sample_index]) : 0.0f };

      LimiterSoundMixComponent::State state;

      state._Boost = _Boost;
      state._Ceiling = _Ceiling;

      float before{ sample };

      LimiterSoundMixComponent::Process(&state, &sample);

      outputs[channel_index][sample_index] = sample;
    }
  }
}
#endif