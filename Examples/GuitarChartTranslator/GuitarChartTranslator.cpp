//Header file.
#include "GuitarChartTranslator.h"

//Core.
#include <Core/General/DynamicString.h>

//IPlug2.
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

/*
* Dropdown menu class definition.
*/
class DropDownMenu final : public IControl
{

public:

  DropDownMenu(GuitarChartTranslator *const RESTRICT plugin, const iplug::igraphics::IRECT& bounds, const int32 parameter_index)
    :
    IControl(bounds, parameter_index)
  {
    _Plugin = plugin;
  }

  void OnMouseDown(float X, float Y, const IMouseMod &mod) override
  {
    IControl::OnMouseDown(X, Y, mod);

    _CurrentHoverState = HoverState::CLICKED;

    Execute();

    SetDirty();
  }

  void OnMouseOver(float X, float Y, const IMouseMod &mod) override
  {
    IControl::OnMouseOver(X, Y, mod);

    if (_CurrentHoverState != HoverState::CLICKED)
    {
      _CurrentHoverState = HoverState::HOVERED;
    }

    SetDirty();
  }

  void OnMouseOut() override
  {
    IControl::OnMouseOut();

    if (_CurrentHoverState != HoverState::CLICKED)
    {
      _CurrentHoverState = HoverState::IDLE;
    }

    SetDirty();
  }

  void Draw(IGraphics &graphics) override
  {
    CalculateName();

    graphics.FillRect(GetButtonColor(), mRECT);
    graphics.DrawText(IText(16, IColor(255, 255, 255, 255)), _Name.Data(), mRECT);
  }

private:

  enum class HoverState
  {
    IDLE,
    HOVERED,
    CLICKED
  };

  GuitarChartTranslator *RESTRICT _Plugin;
  DynamicString _Name;
  HoverState _CurrentHoverState{ HoverState::IDLE };

  void CalculateName()
  {
    switch (static_cast<GuitarChartTranslator::DestinationLibrary>(_Plugin->GetParam(DESTINATION_LIBRARY)->Int()))
    {
      case GuitarChartTranslator::DestinationLibrary::NONE:
      {
        _Name = "Destination Library: None";

        break;
      }

      case GuitarChartTranslator::DestinationLibrary::SHREDDAGE_3_HYDRA:
      {
        _Name = "Destination Library: Shreddage 3: Hydra";

        break;
      }
    }
  }

  IColor GetButtonColor()
  {
    switch (_CurrentHoverState)
    {
      case HoverState::IDLE:
      {
        return IColor(200, 25, 25, 25);
      }

      case HoverState::HOVERED:
      {
        return IColor(200, 75, 75, 75);
      }

      case HoverState::CLICKED:
      {
        return IColor(200, 125, 125, 125);
      }

      default:
      {
        return IColor(200, 25, 25, 25);
      }
    }
  }

  void Execute()
  {
    IPopupMenu &popup_menu{ _Plugin->_DestinationLibraryMenu };

    _Plugin->GetUI()->CreatePopupMenu(*this, popup_menu, mRECT);

    const int chosen_item_index{ popup_menu.GetChosenItemIdx() };

    if (chosen_item_index > -1)
    {
      SetValue(static_cast<double>(chosen_item_index) / static_cast<double>(GuitarChartTranslator::DestinationLibrary::NUMBER_OF_DESTINATION_LIBRARIES));
    }

    if (mMouseIsOver)
    {
      _CurrentHoverState = HoverState::HOVERED;
    }

    else
    {
      _CurrentHoverState = HoverState::IDLE;
    }
  }

};

GuitarChartTranslator::GuitarChartTranslator(const InstanceInfo& info)
  : iplug::Plugin(info, iplug::MakeConfig(NUMBER_OF_PARAMS, 1))
{
  //Set up the parameters.
  GetParam(STRING_1_TUNING)->InitInt("String 1", 0, -24, 24);
  GetParam(STRING_2_TUNING)->InitInt("String 2", 0, -24, 24);
  GetParam(STRING_3_TUNING)->InitInt("String 3", 0, -24, 24);
  GetParam(STRING_4_TUNING)->InitInt("String 4", 0, -24, 24);
  GetParam(STRING_5_TUNING)->InitInt("String 5", 0, -24, 24);
  GetParam(STRING_6_TUNING)->InitInt("String 6", 0, -24, 24);
  GetParam(STRING_7_TUNING)->InitInt("String 7", 0, -24, 24);
  GetParam(STRING_8_TUNING)->InitInt("String 8", 0, -24, 24);
  GetParam(STRING_9_TUNING)->InitInt("String 9", 0, -24, 24);
  GetParam(STRING_10_TUNING)->InitInt("String 10", 0, -24, 24);
  GetParam(DESTINATION_LIBRARY)->InitInt("Destination Library", 0, 0, static_cast<int>(DestinationLibrary::NUMBER_OF_DESTINATION_LIBRARIES), "Destination Library");

  //Construct the destination library menu.
  _DestinationLibraryMenu.Clear();

  _DestinationLibraryMenu.AddItem("None");
  _DestinationLibraryMenu.AddItem("Shreddage 3: Hydra");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    //Enable mouse over events.
    pGraphics->EnableMouseOver(true);

    //Load the font.
    const bool loaded_font{pGraphics->LoadFont("Roboto-Regular", GUITARCHARTTRANSLATOR_FN)};

    if (!loaded_font)
    {
      ASSERT(false, "Oh no...");
    }

    //Attach the panel background.
    pGraphics->AttachBackground(GUITARCHARTTRANSLATOR_BG);

    //Cache the bounds.
    const iplug::igraphics::IRECT bounds{ pGraphics->GetBounds() };

    iplug::igraphics::IVStyle style;

    style.labelText = iplug::igraphics::IText(16, iplug::igraphics::EVAlign::Top, iplug::igraphics::IColor(255, 255, 255, 255));
    style.valueText = iplug::igraphics::IText(24, iplug::igraphics::EVAlign::Bottom, iplug::igraphics::IColor(255, 255, 255, 255));

    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(0).GetHShifted(-192), STRING_1_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(0).GetHShifted(-96), STRING_2_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(0).GetHShifted(0), STRING_3_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(0).GetHShifted(96), STRING_4_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(0).GetHShifted(192), STRING_5_TUNING, "", style));

    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(96).GetHShifted(-192), STRING_6_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(96).GetHShifted(-96), STRING_7_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(96).GetHShifted(0), STRING_8_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(96).GetHShifted(96), STRING_9_TUNING, "", style));
    pGraphics->AttachControl(new iplug::igraphics::IVKnobControl(bounds.GetCentredInside(96).GetVShifted(96).GetHShifted(192), STRING_10_TUNING, "", style));

    pGraphics->AttachControl(new DropDownMenu(this, bounds.GetCentredInside(384).GetVShifted(-96).GetMidVPadded(24), DESTINATION_LIBRARY));
  };
#endif
}

GuitarChartTranslator::~GuitarChartTranslator()
{

}

#if IPLUG_DSP
void GuitarChartTranslator::OnParamChange(int index)
{
  switch (index)
  {
    case STRING_1_TUNING:
    {
      _StringTunings[0] = GetParam(STRING_1_TUNING)->Int();

      break;
    }

    case STRING_2_TUNING:
    {
      _StringTunings[1] = GetParam(STRING_2_TUNING)->Int();

      break;
    }

    case STRING_3_TUNING:
    {
      _StringTunings[2] = GetParam(STRING_3_TUNING)->Int();

      break;
    }

    case STRING_4_TUNING:
    {
      _StringTunings[3] = GetParam(STRING_4_TUNING)->Int();

      break;
    }

    case STRING_5_TUNING:
    {
      _StringTunings[4] = GetParam(STRING_5_TUNING)->Int();

      break;
    }

    case STRING_6_TUNING:
    {
      _StringTunings[5] = GetParam(STRING_6_TUNING)->Int();

      break;
    }

    case STRING_7_TUNING:
    {
      _StringTunings[6] = GetParam(STRING_7_TUNING)->Int();

      break;
    }

    case STRING_8_TUNING:
    {
      _StringTunings[7] = GetParam(STRING_8_TUNING)->Int();

      break;
    }

    case STRING_9_TUNING:
    {
      _StringTunings[8] = GetParam(STRING_9_TUNING)->Int();

      break;
    }

    case STRING_10_TUNING:
    {
      _StringTunings[9] = GetParam(STRING_10_TUNING)->Int();

      break;
    }

    case DESTINATION_LIBRARY:
    {
      _CurrentDestinationLibrary = static_cast<DestinationLibrary>(GetParam(DESTINATION_LIBRARY)->Int());

      break;
    }
  }
}

void GuitarChartTranslator::ProcessBlock(sample** inputs, sample** outputs, int number_of_frames)
{
  for (int32 i{ 0 }; i < number_of_frames; ++i)
  {
    while (!_MidiQueue.Empty())
    {
      iplug::IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > i)
      {
        break;
      }

      Translate(message);

      _MidiQueue.Remove();
    }
  }
}

void GuitarChartTranslator::ProcessMidiMsg(const IMidiMsg &msg)
{
  _MidiQueue.Add(msg);
}

/*
* Translates.
*/
void GuitarChartTranslator::Translate(const iplug::IMidiMsg& message) NOEXCEPT
{
  switch (_CurrentDestinationLibrary)
  {
    case DestinationLibrary::NONE:
    {
      //Do nothing. (:

      break;
    }

    case DestinationLibrary::SHREDDAGE_3_HYDRA:
    {
      const iplug::IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == iplug::IMidiMsg::EStatusMsg::kNoteOn)
      {
        //Calculate the note number.
        int32 note_number;

        switch (message.Channel())
        {
          case 0:
          {
            note_number = 28 + 36 + _StringTunings[0] + message.NoteNumber();

            break;
          }

          case 1:
          {
            note_number = 28 + 31 + _StringTunings[1] + message.NoteNumber();

            break;
          }

          case 2:
          {
            note_number = 28 + 27 + _StringTunings[2] + message.NoteNumber();

            break;
          }

          case 3:
          {
            note_number = 28 + 22 + _StringTunings[3] + message.NoteNumber();

            break;
          }

          case 4:
          {
            note_number = 28 + 17 + _StringTunings[4] + message.NoteNumber();

            break;
          }

          case 5:
          {
            note_number = 28 + 12 + _StringTunings[5] + message.NoteNumber();

            break;
          }

          case 6:
          {
            note_number = 28 + 7 + _StringTunings[6] + message.NoteNumber();

            break;
          }

          case 7:
          default:
          {
            note_number = 28 + _StringTunings[7] + message.NoteNumber();

            break;
          }
        }

        //Calculate the velocity.
        int32 velocity;

        if (message.Velocity() == 127)
        {
          velocity = 119;
        }

        else if (message.Velocity() == 126)
        {
          velocity = 59;
        }

        else
        {
          velocity = 119;
        }

        //Send the output message.
        iplug::IMidiMsg output_message;

        output_message.MakeNoteOnMsg(note_number, velocity, 0);

        SendMidiMsg(output_message);
      }

      else if (status == iplug::IMidiMsg::EStatusMsg::kNoteOff)
      {
        //Calculate the note number.
        int32 note_number;

        switch (message.Channel())
        {
          case 0:
          {
            note_number = 28 + 36 + _StringTunings[0] + message.NoteNumber();

            break;
          }

          case 1:
          {
            note_number = 28 + 31 + _StringTunings[1] + message.NoteNumber();

            break;
          }

          case 2:
          {
            note_number = 28 + 27 + _StringTunings[2] + message.NoteNumber();

            break;
          }

          case 3:
          {
            note_number = 28 + 22 + _StringTunings[3] + message.NoteNumber();

            break;
          }

          case 4:
          {
            note_number = 28 + 17 + _StringTunings[4] + message.NoteNumber();

            break;
          }

          case 5:
          {
            note_number = 28 + 12 + _StringTunings[5] + message.NoteNumber();

            break;
          }

          case 6:
          {
            note_number = 28 + 7 + _StringTunings[6] + message.NoteNumber();

            break;
          }

          case 7:
          default:
          {
            note_number = 28 + _StringTunings[7] + message.NoteNumber();

            break;
          }
        }

        //Send the output message.
        iplug::IMidiMsg output_message;

        output_message.MakeNoteOffMsg(note_number, 0);

        SendMidiMsg(output_message);
      }

      break;
    }
  }
}
#endif