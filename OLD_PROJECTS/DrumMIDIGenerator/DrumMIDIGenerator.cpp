//Header file.
#include "DrumMIDIGenerator.h"

//Math.
#include <Math/Core/CatalystRandomMath.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

DrumMIDIGenerator::DrumMIDIGenerator(const iplug::InstanceInfo& info)
: iplug::Plugin(info, iplug::MakeConfig(0, 1))
{
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
    pGraphics->LoadFont("Default-VST-Font", DRUMMIDIGENERATOR_FN);

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::COLOR_DARK_GRAY);
    
    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the title text.
    {
      _Text = new iplug::igraphics::ITextControl(bounds.GetCentredInside(512).GetVShifted(-128), "Drum MIDI Generator", iplug::igraphics::IText(64));

      pGraphics->AttachControl(_Text);
    }
  };
#endif

  //Open the output log.
  _OutputLog.open("Drum MIDI Generator Output Log.txt");
}


DrumMIDIGenerator::~DrumMIDIGenerator()
{
  _OutputLog.close();
}

#if IPLUG_DSP
void DrumMIDIGenerator::OnReset()
{
  //_OutputLog << "OnReset()";
}


void DrumMIDIGenerator::OnActivate(bool value)
{
  _OutputLog << (value ? "OnActivate true" : "OnActivate false") << std::endl;
}

void DrumMIDIGenerator::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  static int32 frame_count{ 0 };

  if (!mTimeInfo.mTransportIsRunning)
  {
    return;
  }

  for (int32 i{ 0 }; i < number_of_frames; ++i)
  {
    ++frame_count;

    if (frame_count >= 1'024 * 8)
    {
      iplug::IMidiMsg message;

      message.MakeNoteOnMsg(24, 127, 0);

      if (SendMidiMsg(message))
      {
        //_OutputLog << "Successfully sent MIDI message.";
      }

      else
      {
        //_OutputLog << "Failed to send MIDI message.";
      }

      frame_count = 0;
    }
  }
}

void DrumMIDIGenerator::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  
}
#endif