#include "DaybreakDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

DaybreakDistortion::DaybreakDistortion(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
  GetParam(kGain)->InitDouble("Distortion", 0.0, 0.0, 100.0, 0.01, "%");

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_DARK_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Distortion yaaas", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
  };
#endif
}

#if IPLUG_DSP
void DaybreakDistortion::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  const sample distortion{ 1.0 - (GetParam(kGain)->Value() / 100.0) };
  const int number_of_channels{ NOutChansConnected() };
  
  for (int i{ 0 }; i < number_of_frames; ++i)
  {
    for (int j{ 0 }; j < number_of_channels; ++j)
    {
      if (inputs[j][i] >= 0.0)
      {
        outputs[j][i] = inputs[j][i] < distortion ? inputs[j][i] : distortion;
      }

      else
      {
        outputs[j][i] = inputs[j][i] > -distortion ? inputs[j][i] : -distortion;
      }

      outputs[j][i] = distortion > 0.0 ? outputs[j][i] / distortion : outputs[j][i] * 100.0;
    }
  }
}
#endif
