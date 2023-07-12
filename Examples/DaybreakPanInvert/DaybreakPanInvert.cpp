#include "DaybreakPanInvert.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

DaybreakPanInvert::DaybreakPanInvert(const InstanceInfo& info)
: Plugin(info, MakeConfig(0, 1))
{
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Daybreak Pan Invert", IText(50)));
  };
#endif
}

#if IPLUG_DSP
void DaybreakPanInvert::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  const int number_of_channels{ NOutChansConnected() };
  
  for (int frame{ 0 }; frame < number_of_frames; ++frame)
  {
    for (int channel{ 0 }; channel < number_of_channels; ++channel)
    {
      if (number_of_channels == 2)
      {
        switch (channel)
        {
          case 0:
          {
            outputs[channel][frame] = inputs[1][frame];

            break;
          }

          case 1:
          {
            outputs[channel][frame] = inputs[0][frame];

            break;
          }
        }
      }

      else
      {
        outputs[channel][frame] = inputs[channel][frame];
      }
    }
  }
}
#endif
