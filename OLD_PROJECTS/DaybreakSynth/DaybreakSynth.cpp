#include "DaybreakSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

//Math.
#include <Math/Core/CatalystBaseMath.h>
#include <Math/Core/CatalystRandomMath.h>

//Resources.
#include <Resources/Reading/SoundResourceReader.h>

DaybreakSynth::DaybreakSynth(const InstanceInfo& info)
: Plugin(info, MakeConfig(NUMBER_OF_PARAMS, kNumPrograms))
{
  GetParam(PRE_GAIN)->InitDouble("PRE GAIN", 10.0, 0.0, 100.0, 0.01, "%");
  GetParam(WAVE_TYPE)->InitEnum("WAVE TYPE", 0, 4);
  GetParam(WHITE_NOISE)->InitDouble("WHITE NOISE", 0.0, 0.0, 100.0, 0.01, "%");
  GetParam(DISTORTION)->InitDouble("DISTORTION", 0.0, 0.0, 99.99, 0.01, "%");
  GetParam(ATTACK_VALUE)->InitDouble("ATTACK", 0.01, 0.01, 10.0, 0.01);
  GetParam(DECAY_VALUE)->InitDouble("DECAY", 0.01, 0.01, 10.0, 0.01);
  GetParam(SUSTAIN_VALUE)->InitDouble("SUSTAIN", 1.0, 0.01, 10.0, 0.01);
  GetParam(RELEASE_VALUE)->InitDouble("RELEASE", 0.01, 0.01, 10.0, 0.01);

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_DARK_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50).GetVShifted(-220), "synth yaaas", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-125).GetHShifted(-100), PRE_GAIN));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-125).GetHShifted(100), WAVE_TYPE));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(0).GetHShifted(-100), WHITE_NOISE));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(0).GetHShifted(100), DISTORTION));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(150).GetHShifted(-150), ATTACK_VALUE));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(150).GetHShifted(-50), DECAY_VALUE));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(150).GetHShifted(50), SUSTAIN_VALUE));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(150).GetHShifted(150), RELEASE_VALUE));
  };
#endif
}

#if IPLUG_DSP
void DaybreakSynth::OnReset()
{
  for (PlayingNote& playing_note : _PlayingNotes)
  {
    playing_note._Oscillator.SetSampleRate(GetSampleRate());
    playing_note._ADSREnvelope.SetSampleRate(GetSampleRate());
  }
}

void DaybreakSynth::OnParamChange(const int index)
{
  switch (index)
  {
    case WAVE_TYPE:
    {
      _WaveType = static_cast<Oscillator::OscillatorMode>(static_cast<int>(GetParam(WAVE_TYPE)->Value()));

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        playing_note._Oscillator.SetMode(_WaveType);
      }

      break;
    }
  }
}

void DaybreakSynth::ProcessBlock(sample** inputs, sample** outputs, const int32 number_of_frames)
{
  const int number_of_channels{ NOutChansConnected() };
  
  for (int i{ 0 }; i < number_of_frames; ++i)
  {
    //Figure out the pure value. (:
    while (!_MidiQueue.Empty())
    {
      IMidiMsg &message{ _MidiQueue.Peek() };

      if (message.mOffset > i)
      {
        break;
      }

      const IMidiMsg::EStatusMsg status{ message.StatusMsg() };

      if (status == IMidiMsg::kNoteOn)
      {
        _PlayingNotes.EmplaceSlow();
        PlayingNote &playing_note{ _PlayingNotes.Back() };

        playing_note._Oscillator.SetMode(_WaveType);
        playing_note._Oscillator.SetFrequency(440.0f * powf(2.0f, (message.mData1 - 69.0f) / 12.0f));
        playing_note._Oscillator.SetSampleRate(GetSampleRate());
        playing_note._ADSREnvelope.SetSampleRate(GetSampleRate());
        playing_note._ADSREnvelope.SetStageValues(GetParam(ATTACK_VALUE)->Value(), GetParam(DECAY_VALUE)->Value(), GetParam(SUSTAIN_VALUE)->Value(), GetParam(RELEASE_VALUE)->Value());
        playing_note._ADSREnvelope.EnterAttackStage();
        playing_note._NoteNumber = message.mData1;
      }

      else if (status == IMidiMsg::kNoteOff)
      {
        for (PlayingNote& playing_note : _PlayingNotes)
        {
          if (playing_note._NoteNumber == message.mData1)
          {
            playing_note._ADSREnvelope.EnterReleaseStage();
          }
        }
      }

      _MidiQueue.Remove();
    }

    float value{ 0.0f };

    for (PlayingNote& playing_note : _PlayingNotes)
    {
      value += playing_note._Oscillator.Generate() * playing_note._ADSREnvelope.Update();
    }

    //Check if any playing notes should be removed.
    bool note_was_removed{ false };

    do
    {
      note_was_removed = false;

      for (PlayingNote& playing_note : _PlayingNotes)
      {
        if (!playing_note._ADSREnvelope.IsActive())
        {
          playing_note = _PlayingNotes.Back();
          _PlayingNotes.PopFast();

          note_was_removed = true;

          break;
        }
      }
    } while (note_was_removed);
    

    //Apply white noise!
    const float white_noise{ static_cast<float>(GetParam(WHITE_NOISE)->Value()) / 100.0f };

    if (!_PlayingNotes.Empty() && white_noise > 0.0f)
    {
      value = CatalystBaseMath::LinearlyInterpolate(value, CatalystRandomMath::RandomFloatInRange(-1.0f, 1.0f), white_noise);
    }

    //Apply pre gain!
    const float pre_gain{ static_cast<float>(GetParam(PRE_GAIN)->Value()) / 100.0f };

    value *= pre_gain;

    //Apply distortion!
    const float distortion{ 1.0f - (static_cast<float>(GetParam(DISTORTION)->Value()) / 100.0f) };

    if (value >= 0.0)
    {
      value = std::min(value, distortion);
    }

    else
    {
      value = std::max(value, -distortion);
    }

    value = distortion > 0.0f ? value / distortion : value;

    for (int j{ 0 }; j < number_of_channels; ++j)
    {
      outputs[j][i] = value;
    }
  }
}

void DaybreakSynth::ProcessMidiMsg(const IMidiMsg& msg)
{
  _MidiQueue.Add(msg);
}
#endif
