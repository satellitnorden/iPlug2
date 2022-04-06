//Header file.
#include "ProceduralMusician.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//Procedural Musician.
#include "FirstSectionGenerator.h"
#include "SecondSectionGenerator.h"
#include "ThirdSectionGenerator.h"
#include "FourthSectionGenerator.h"

ProceduralMusician::ProceduralMusician(const iplug::InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Add all the section generators.
  _SectionGenerators.Emplace(new FirstSectionGenerator());
  _SectionGenerators.Emplace(new SecondSectionGenerator());
  _SectionGenerators.Emplace(new ThirdSectionGenerator());
  _SectionGenerators.Emplace(new FourthSectionGenerator());

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
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", PROCEDURALMUSICIAN_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    pGraphics->AttachPanelBackground(iplug::igraphics::IColor(25, 25, 25, 25));

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the "Next Section" button.
    {
      iplug::igraphics::IVButtonControl* const RESTRICT button{ new iplug::igraphics::IVButtonControl(bounds.GetCentredInside(128), [this](iplug::igraphics::IControl*) { _CurrentSectionGenerator = (_CurrentSectionGenerator + 1) % _SectionGenerators.Size(); }, "Next Section") };

      pGraphics->AttachControl(button);
    }
  };
#endif

#if USE_OUTPUT_LOG
  //Open the output log.
  char buffer[MAX_PATH];
  sprintf_s(buffer, "Procedural Musician Output Log.txt");

  _OutputLog.open(buffer);
#endif
}

ProceduralMusician::~ProceduralMusician()
{
  //Deallocate all section generators.
  for (SectionGenerator *const RESTRICT section_generator: _SectionGenerators)
  {
    delete section_generator;
  }

#if USE_OUTPUT_LOG
  //Close the output log.
  _OutputLog.close();
#endif
}

#if IPLUG_DSP
void ProceduralMusician::OnReset()
{
  
}

void ProceduralMusician::OnParamChange(const int32 index)
{
  
}

void ProceduralMusician::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int32 number_of_frames)
{
  for (int32 sample_index{ 0 }; sample_index < number_of_frames; ++sample_index)
  {
    //Process incomoing MIDI messages.
    while (!_IncomingMidiQueue.Empty())
    {
      const iplug::IMidiMsg& message{ _IncomingMidiQueue.Peek() };

      if (message.mOffset > sample_index)
      {
        break;
      }

      if (message.StatusMsg() == iplug::IMidiMsg::kNoteOn)
      {
        OnMIDIEventContext context;

        context._BeatsPerMinute = GetTempo();
        context._SampleRate = GetSampleRate();
        context._NoteNumber = message.NoteNumber();
        context._MIDIEvents = &_OutgoingMidiQueue;

        GetCurrentSectionGenerator()->OnMIDIEvent(context);
      }

      _IncomingMidiQueue.Remove();
    }

    //Process outgoing MIDI messages.
    if (!_OutgoingMidiQueue.Empty())
    {
      for (uint64 i{ 0 }; i < _OutgoingMidiQueue.Size();)
      {
        --_OutgoingMidiQueue[i]._Offset;

        if (_OutgoingMidiQueue[i]._Offset <= 0)
        {
          iplug::IMidiMsg message{ _OutgoingMidiQueue[i]._Message };
          message.mOffset = sample_index;

          SendMidiMsg(message);

          _OutgoingMidiQueue.EraseAt<true>(i);
        }

        else
        {
          ++i;
        }
      }
    }
  }
}

void ProceduralMusician::ProcessMidiMsg(const iplug::IMidiMsg& msg)
{
  _IncomingMidiQueue.Add(msg);
}
#endif