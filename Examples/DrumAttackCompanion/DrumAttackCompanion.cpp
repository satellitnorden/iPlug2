//Header file.
#include "DrumAttackCompanion.h"

//IPlug2.
#include "IPlug_include_in_plug_src.h"

class IndexedTextControl final : public iplug::igraphics::ITextControl
{

public:

  int32 _Index;

  IndexedTextControl(const int32 index, const IRECT& bounds, const char* str = "", const IText& text = DEFAULT_TEXT, const IColor& BGColor = DEFAULT_BGCOLOR, bool setBoundsBasedOnStr = false)
    :
    iplug::igraphics::ITextControl(bounds, str, text, BGColor, setBoundsBasedOnStr),
    _Index(index)
  {

  }

};

DrumAttackCompanion::DrumAttackCompanion(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{   
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Load the font.
    const bool loaded_font{ pGraphics->LoadFont("Roboto-Regular", DRUMATTACKCOMPANION_FN) };

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the corner resizer.
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);

    //Attach the panel background.
    //pGraphics->AttachBackground(DRUMATTACKCOMPANION_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    //Create the texts.
    for (int32 i{ 0 }; i < 5; ++i)
    {
      const iplug::igraphics::IRECT position{ bounds.GetCentredInside(512).GetVShifted(-245 + (24 * i)) };

      const char* RESTRICT string;

      switch (i)
      {
        case 0:
        {
          string = "1. Rhythm >";

          break;
        }

        case 1:
        {
          string = "2. Rhythm Lofi >";

          break;
        }

        case 2:
        {
          string = "3. Lead >";

          break;
        }

        case 3:
        {
          string = "4. Lead Lofi >";

          break;
        }

        case 4:
        {
          string = "5. Clean >";

          break;
        }
       }

      const iplug::igraphics::IText text{ iplug::igraphics::IText(16, iplug::igraphics::IColor(255, 255 / 2, 255 / 2, 255 / 2), nullptr, EAlign::Far, EVAlign::Middle) };

      _Texts.Emplace(new IndexedTextControl(i, position, string, text));

      _Texts.Back()->SetAnimation
      (
        [this](IControl *const RESTRICT control)
        {
          if (_CurrentChannel == static_cast<IndexedTextControl* const RESTRICT>(control)->_Index)
          {
            static_cast<IndexedTextControl* const RESTRICT>(control)->SetText(iplug::igraphics::IText(16, iplug::igraphics::IColor(255, 255, 255, 255), nullptr, EAlign::Far, EVAlign::Middle));
          }

          else
          {
            static_cast<IndexedTextControl* const RESTRICT>(control)->SetText(iplug::igraphics::IText(16, iplug::igraphics::IColor(255, 255 / 2, 255 / 2, 255 / 2), nullptr, EAlign::Far, EVAlign::Middle));
          }
        }
      );

      pGraphics->AttachControl(_Texts.Back());
    }
  };
#endif
}

DrumAttackCompanion::~DrumAttackCompanion()
{
  
}

#if IPLUG_DSP
void DrumAttackCompanion::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  //Cache the number of channels.
  const int number_of_channels{ NOutChansConnected() };

  for (int frame_index{ 0 }; frame_index < number_of_frames; ++frame_index)
  {
    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > frame_index)
      {
        break;
      }

      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::kNoteOn)
      {
        //Extract the note number and velocity.
        const int note_number{ message.mData1 };
        const int velocity{ message.mData2 };

        if (note_number == 36)
        {
          _CurrentChannel = velocity - 1;
        }
      }

      _MidiQueue.Remove();
    }

    for (int32 channel_index{ 0 }; channel_index < number_of_channels; ++channel_index)
    {
      if (_CurrentChannel == channel_index)
      {
        outputs[channel_index][frame_index] = inputs[0][frame_index];
      }

      else
      {
        outputs[channel_index][frame_index] = 0.0f;
      }
    }
  }
}

void DrumAttackCompanion::ProcessMidiMsg(const IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}
#endif