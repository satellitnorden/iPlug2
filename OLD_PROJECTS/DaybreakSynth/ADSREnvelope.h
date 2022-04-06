#pragma once

#include <array>

class ADSREnvelope
{

public:

  //Enumeration covering all stages.
  enum class Stage
  {
    STAGE_OFF,
    STAGE_ATTACK,
    STAGE_DECAY,
    STAGE_SUSTAIN,
    STAGE_RELEASE
  };

  /*
  * Default constructor.
  */
  ADSREnvelope()
  {
    _StageValues[0] = 0.01f; //ATTACK TIME
    _StageValues[1] = 0.01f; //DECAY TIME
    _StageValues[2] = 0.9f; //SUSTAIN GAIN
    _StageValues[3] = 0.01f; //RELEASE TIME
  }

  /*
  * Sets the sample rate.
  */
  void SetSampleRate(const float sample_rate)
  {
    _SampleRate = sample_rate;
  }

  /*
  * Sets the stage values.
  */
  void SetStageValues(const float attack, const float decay, const float sustain, const float release)
  {
    _StageValues[0] = attack;
    _StageValues[1] = decay;
    _StageValues[2] = sustain;
    _StageValues[3] = release;
  }

  /*
  * Returns if this envelope is active.
  */
  bool IsActive()
  {
    return _CurrentStage != Stage::STAGE_OFF;
  }

  /*
  * Enters the OFF stage for this envelope.
  */
  void EnterOffStage()
  {
    _CurrentStage = Stage::STAGE_OFF;
    _CurrentSample = 0;
    _SamplesUntilNextStage = 0;
  }

  /*
  * Enters the ATTACK stage for this envelope.
  */
  void EnterAttackStage()
  {
    _CurrentStage = Stage::STAGE_ATTACK;
    _CurrentSample = 0;
    _SamplesUntilNextStage = static_cast<unsigned int>(_StageValues[0] * _SampleRate);
  }

  /*
  * Enters the DECAY stage for this envelope.
  */
  void EnterDecayStage()
  {
    _CurrentStage = Stage::STAGE_DECAY;
    _CurrentSample = 0;
    _SamplesUntilNextStage = static_cast<unsigned int>(_StageValues[1] * _SampleRate);
  }

  /*
  * Enters the SUSTAIN stage for this envelope.
  */
  void EnterSustainStage()
  {
    _CurrentStage = Stage::STAGE_SUSTAIN;
    _CurrentSample = 0;
    _SamplesUntilNextStage = 0;
  }

  /*
  * Enters the RELEASE stage for this envelope.
  */
  void EnterReleaseStage()
  {
    _CurrentStage = Stage::STAGE_RELEASE;
    _CurrentSample = 0;
    _SamplesUntilNextStage = static_cast<unsigned int>(_StageValues[3] * _SampleRate);
  }

  /*
  * Updates this envelope, returning the current gain.
  */
  float Update()
  {
    switch (_CurrentStage)
    {
      case Stage::STAGE_OFF:
      {
        return 0.0f;
      }

      case Stage::STAGE_ATTACK:
      {
        //Calculate the multiplier.
        const float multiplier{ static_cast<float>(_CurrentSample) / static_cast<float>(_SamplesUntilNextStage) };

        //Advance the current sample.
        ++_CurrentSample;

        //Check if the decay stage should be entered.
        if (_CurrentSample >= _SamplesUntilNextStage)
        {
          EnterDecayStage();
        }

        //For the attack stage, the gain is simply the multiplier.
        return multiplier;
      }

      case Stage::STAGE_DECAY:
      {
        //Calculate the multiplier.
        const float multiplier{ static_cast<float>(_CurrentSample) / static_cast<float>(_SamplesUntilNextStage) };

        //Advance the current sample.
        ++_CurrentSample;

        //Check if the sustain stage should be entered.
        if (_CurrentSample >= _SamplesUntilNextStage)
        {
          EnterSustainStage();
        }

        //For the decay stage, blend in the sustain value.
        return LinearlyInterpolate(1.0f, _StageValues[2], multiplier);
      }

      case Stage::STAGE_SUSTAIN:
      {
        //For the sustain stage, simply return the sustain value.
        return _StageValues[2];
      }

      case Stage::STAGE_RELEASE:
      {
        //Calculate the multiplier.
        const float multiplier{ static_cast<float>(_CurrentSample) / static_cast<float>(_SamplesUntilNextStage) };

        //Advance the current sample.
        ++_CurrentSample;

        //Check if the off stage should be entered.
        if (_CurrentSample >= _SamplesUntilNextStage)
        {
          EnterOffStage();
        }

        //For the decay stage, blend in the sustain value.
        return LinearlyInterpolate(_StageValues[2], 0.0f, multiplier);
      }

      default:
      {
        return 0.0f;
      }
    }
  }

private:

  //The current stage.
  Stage _CurrentStage{ Stage::STAGE_OFF };

  //The stage values. Represents time for ATTACK, DECAY and RELEASE, and gain for SUSTAIN.
  std::array<float, 4> _StageValues;

  //The sample rate.
  float _SampleRate{ 48'000.0f };

  //The current sample.
  unsigned int _CurrentSample{ 0 };

  //The number of samples until the next stage.
  unsigned int _SamplesUntilNextStage{ 0 };

  /*
  * Linearly interpolates between two values.
  */
  float LinearlyInterpolate(const float A, const float B, const float alpha)
  {
    return (A * (1.0f - alpha)) + (B * alpha);
  }

};