#include "AutoGainStager.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"


//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

AutoGainStager::AutoGainStager(const InstanceInfo& info)
: Plugin(info, MakeConfig(0, 1))
{
#if IPLUG_EDITOR
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
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };
  };
#endif
}

#if IPLUG_DSP
void AutoGainStager::OnReset()
{
  
}

void AutoGainStager::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  //Just copy the inputs, for now.
  const int32 number_of_channels{ NOutChansConnected() };

  for (int32 frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
    for (int32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      outputs[channel_index][frame_index] = inputs ? inputs[channel_index][frame_index] : 0.0f;
    }
  }
}

void AutoGainStager::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  
}
#endif

bool AutoGainStager::SerializeState(iplug::IByteChunk& chunk) const
{
  const bool parent_succeeded{ iplug::Plugin::SerializeState(chunk) };

  return parent_succeeded;
}

int AutoGainStager::UnserializeState(const iplug::IByteChunk& chunk, int startPos)
{
  const int parent_pos{ iplug::Plugin::UnserializeState(chunk, startPos) };

  return parent_pos;
}