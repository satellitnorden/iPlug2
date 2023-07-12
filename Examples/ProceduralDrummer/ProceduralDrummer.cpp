//Header file.
#include "ProceduralDrummer.h"

//iPlug.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

ProceduralDrummer::ProceduralDrummer(const InstanceInfo& info)
  : Plugin(info, MakeConfig(0, 1))
{
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics)
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
      pGraphics->AttachControl(new iplug::igraphics::ITextControl(bounds.GetCentredInside(512.0f).GetVShifted(-128.0f), "Procedural Drummer", iplug::igraphics::IText(64)));
    }
  };

  //Retrieve the preliminary sample rate.
  _SampleRate = static_cast<uint64>(GetSampleRate());
#endif
}

#if IPLUG_DSP
void ProceduralDrummer::OnReset()
{
  //Retrieve the sample rate.
  _SampleRate = static_cast<uint64>(GetSampleRate());
}

void ProceduralDrummer::ProcessBlock(iplug::sample **inputs, iplug::sample **outputs, int32 number_of_frames)
{
  for (int32 frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
    while (!_IncomingMidiQueue.Empty())
    {
      iplug::IMidiMsg &input_message{ _IncomingMidiQueue.Peek() };

      if (input_message.mOffset > frame_index)
      {
        break;
      }

      if (input_message.StatusMsg() == iplug::IMidiMsg::kNoteOn)
      {
        if (GetTransportIsRunning())
        {
          //Trigger the kick. (:
          iplug::IMidiMsg output_message;
          output_message.MakeNoteOnMsg(24, 127, frame_index);

          SendMidiMsg(output_message);
        }
      }

      _IncomingMidiQueue.Remove();
    }

    if (GetTransportIsRunning())
    {
      if (static_cast<uint64>(GetSamplePos()) % static_cast<uint64>(GetSamplesPerBeat()) == 0)
      {
        //Trigger the snare. (:
        iplug::IMidiMsg output_message;
        output_message.MakeNoteOnMsg(26, 127, frame_index);

        SendMidiMsg(output_message);
      }
    }
  }
}

void ProceduralDrummer::ProcessMidiMsg(const iplug::IMidiMsg &message)
{
  _IncomingMidiQueue.Add(message);
}
#endif
