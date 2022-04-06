//Header file.
#include "MIDIGate.h"

//Math.
#include <Math/Core/CatalystBaseMath.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

MIDIGate::MIDIGate(const iplug::InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
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
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", MIDIGATE_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachBackground(MIDIGATE_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the info text.
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Open Notes: 100-127", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-96), "Muted Notes: 90-99", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-64), "Tapped Open Notes: 80-89", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-32), "Tapped Muted notes : 70-79", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
    pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(0), "Dead notes : 60-69", iplug::igraphics::IText(24, iplug::igraphics::IColor(255, 255, 255, 255))));
  };
#endif
}

MIDIGate::~MIDIGate()
{

}

#if IPLUG_DSP
void MIDIGate::OnReset()
{
  
}

void MIDIGate::OnParamChange(const int32 index)
{
  
}

void MIDIGate::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  const int32 number_of_channels{ NOutChansConnected() };

  for (int32 frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
    //Count the number of MIDI notes playing.
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg& message{ _MidiQueue.Peek() };

      if (message.mOffset > frame_index)
      {
        break;
      }

      if (message.StatusMsg() == iplug::IMidiMsg::EStatusMsg::kNoteOn)
      {
        ++_NumberOfMIDINotesPlaying;
      }

      else if (message.StatusMsg() == iplug::IMidiMsg::EStatusMsg::kNoteOff)
      {
        --_NumberOfMIDINotesPlaying;
      }

      _MidiQueue.Remove();
    }

    //Update the current gate value.
    if (_NumberOfMIDINotesPlaying > 0)
    {
      _CurrentGateValue = CatalystBaseMath::Minimum<int32>(_CurrentGateValue + 1, MIDI_GATE_BUFFER_SIZE);
    }

    else
    {
      _CurrentGateValue = CatalystBaseMath::Maximum<int32>(_CurrentGateValue - 1, 0);
    }

    for (int32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      if (inputs)
      {
        SetCurrentBufferValue(channel_index, inputs[channel_index][frame_index]);
        outputs[channel_index][frame_index] = GetPreviousBufferValue(channel_index) * CatalystBaseMath::SmoothStep<1>(static_cast<iplug::sample>(_CurrentGateValue) / static_cast<iplug::sample>(MIDI_GATE_BUFFER_SIZE));
      }
    }

    _CurrentBufferIndex = (_CurrentBufferIndex + 1) & (MIDI_GATE_BUFFER_SIZE - 1);
  }
}

void MIDIGate::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}
#endif