#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

class SamplesConform final
{

public:

  /*
  * Conforms Pangaea drum kit samples.
  */
  FORCE_INLINE static void Pangaea() NOEXCEPT
  {
    //Conform kick.
    {
      constexpr const char *const RESTRICT INPUT_MICROPHONES[]
      {
        "BETA 52",
        "D112",
        "OVERHEAD"
      };

      constexpr const char *const RESTRICT OUTPUT_MICROPHONES[]
      {
        "1",
        "2",
        "OVERHEAD"
      };

      PangaeaInternal(5, 3, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "SHELLS\\KICK", "KICK");
    }

    // Conform snare.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]
      {
        "SM57",
        "MD421",
        "BLUE MOUSE",
        "OVERHEAD"
      };

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"TOP_1", "TOP_2", "BOTTOM", "OVERHEAD"};

      PangaeaInternal(6, 4, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "SHELLS\\SNARE", "SNARE");
    }

    // Conform tom 1.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"MD421", "D112", "OVERHEAD"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"1", "2", "OVERHEAD"};

      PangaeaInternal(4, 3, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "SHELLS\\TOM 1", "TOM_1");
    }

    // Conform tom 2.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"MD421", "D112", "OVERHEAD"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"1", "2", "OVERHEAD"};

      PangaeaInternal(4, 3, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "SHELLS\\TOM 2", "TOM_2");
    }

    // Conform tom 3.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"MD421", "D112", "OVERHEAD"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"1", "2", "OVERHEAD"};

      PangaeaInternal(4, 3, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "SHELLS\\TOM 3", "TOM_3");
    }

    // Conform china.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(4, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\CHINA", "CHINA");
    }

    // Conform left crash.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(4, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\LEFT CRASH", "LEFT_CRASH");
    }

    // Conform right crash.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(4, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\RIGHT CRASH", "RIGHT_CRASH");
    }

    // Conform right stack.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(4, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\STACK", "STACK");
    }

    // Conform ride bow.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(4, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\RIDE\\BOW", "RIDE_BOW");
    }

    // Conform ride bell.
    {
      constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

      constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

      PangaeaInternal(2, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\RIDE\\BELL", "RIDE_BELL");
    }

    // Conform hihat samples.
    {
      {
        constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

        constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

        PangaeaInternal(1, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, "CYMBALS\\HI-HAT\\FOOT", "HIHAT_PEDAL");
      }

      for (uint8 layer_index{0}; layer_index < 8; ++layer_index)
      {
        constexpr const char* const RESTRICT INPUT_MICROPHONES[]{"BLUE MOUSE"};

        constexpr const char* const RESTRICT OUTPUT_MICROPHONES[]{"CLOSE_MIC"};

        char buffer_1[MAXIMUM_FILE_PATH_LENGTH];
        sprintf_s(buffer_1, "CYMBALS\\HI-HAT\\LAYER %u", 8 - layer_index);

        char buffer_2[MAXIMUM_FILE_PATH_LENGTH];
        sprintf_s(buffer_2, "HIHAT_LEVEL_%u", layer_index);

        PangaeaInternal(1, 1, INPUT_MICROPHONES, OUTPUT_MICROPHONES, buffer_1, buffer_2);
      }

    }
  }

private:

  /*
  * Conforms Pangaea drum kit samples.
  */
  FORCE_INLINE static void PangaeaInternal( const uint8 number_of_velocity_layers,
                                            const uint8 number_of_microphones,
                                            const char *const RESTRICT *const RESTRICT input_microphones,
                                            const char *const RESTRICT *const RESTRICT output_microphones,
                                            const char *const RESTRICT sub_directory,
                                            const char *const RESTRICT piece_name) NOEXCEPT
  {
    constexpr char* const RESTRICT INPUT_DIRECTORY{"C:\\Pangaea Drum Kit Samples Export"};
    constexpr char* const RESTRICT OUTPUT_DIRECTORY{"C:\\Pangaea Drum Kit Samples"};

    for (uint8 velocity_layer_index{ 0 }; velocity_layer_index < number_of_velocity_layers; ++velocity_layer_index)
    {
      for (uint8 microphone_index{ 0 }; microphone_index < number_of_microphones; ++microphone_index)
      {
        const char *const RESTRICT input_microphone{input_microphones[microphone_index]};
        const char *const RESTRICT output_microphone{output_microphones[microphone_index]};

        char input_directory_buffer[MAXIMUM_FILE_PATH_LENGTH];

        if (number_of_velocity_layers == 1)
        {
          sprintf_s(input_directory_buffer, "%s\\%s\\%s", INPUT_DIRECTORY, sub_directory, input_microphone);
        }

        else
        {
          sprintf_s(input_directory_buffer, "%s\\%s\\VELOCITY LAYER %u\\%s", INPUT_DIRECTORY, sub_directory, number_of_velocity_layers - velocity_layer_index, input_microphone);
        }

        uint32 variation_index{ 0 };

        for (const auto &entry : std::filesystem::directory_iterator(std::string(input_directory_buffer)))
        {
          char output_file_buffer[MAXIMUM_FILE_PATH_LENGTH];

          sprintf_s(output_file_buffer, "%s\\%s_%s_%u_%u.wav", OUTPUT_DIRECTORY, piece_name, output_microphone, velocity_layer_index, variation_index);

          std::filesystem::copy(entry.path(), std::string(output_file_buffer), std::filesystem::copy_options::overwrite_existing);

          ++variation_index;
        }
      }
    }
  }

};