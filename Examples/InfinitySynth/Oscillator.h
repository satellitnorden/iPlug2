#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

class Oscillator
{

public:

  enum class OscillatorMode
  {
    SINE,
    SAW,
    SQUARE,
    TRIANGLE
  };

  /*
  * Default constructor.
  */
  Oscillator()
  {
    CalculatePhaseIncrement();
  }

  /*
  * Sets the mode of this oscillator.
  */
  void SetMode(const OscillatorMode mode)
  {
    _Mode = mode;
  }

  /*
  * Sets the frequency.
  */
  void SetFrequency(const float frequency)
  {
    _Frequency = frequency;

    CalculatePhaseIncrement();
  }

  /*
  * Sets the sample rate.
  */
  void SetSampleRate(const float sample_rate)
  {
    _SampleRate = sample_rate;

    CalculatePhaseIncrement();
  }

  /*
  * Generates the next sample.
  */
  float Generate()
  {
    constexpr float DOUBLE_PI{ static_cast<float>(M_PI) * 2.0f };

    float value{ 0.0f };

    switch (_Mode)
    {
      case OscillatorMode::SINE:
      {
        value = sin(_CurrentPhase);

        break;
      }

      case OscillatorMode::SAW:
      {
        value = 1.0f - (2.0f * _CurrentPhase / DOUBLE_PI);

        break;
      }

      case OscillatorMode::SQUARE:
      {
        if (_CurrentPhase <= M_PI)
        {
          value = 1.0f;
        }

        else
        {
          value = -1.0f;
        }

        break;
      }

      case OscillatorMode::TRIANGLE:
      {
        value = -1.0f + (2.0f * _CurrentPhase / DOUBLE_PI);
        value = 2.0f * (fabs(value) - 0.5f);

        break;
      }
    }

    ExecutePhaseIncrement();

    return value;
  }

private:

  //The mode.
  OscillatorMode _Mode{ OscillatorMode::SINE };

  //The frequency.
  float _Frequency{ 440.0f };

  //The sample rate.
  float _SampleRate{ 48'000.0f };

  //The current phase.
  float _CurrentPhase{ 0.0f };

  //The phase increment.
  float _PhaseIncrement{ 0.0f };

  /*
  * Calculates the phase increment.
  */
  void CalculatePhaseIncrement()
  {
    _PhaseIncrement = _Frequency * 2.0f * M_PI / _SampleRate;
  }

  /*
  * Executes the phase increment.
  */
  void ExecutePhaseIncrement()
  {
    constexpr float DOUBLE_PI{ M_PI * 2.0f };

    _CurrentPhase += _PhaseIncrement;

    while (_CurrentPhase >= DOUBLE_PI)
    {
      _CurrentPhase -= DOUBLE_PI;
    }
  }

};