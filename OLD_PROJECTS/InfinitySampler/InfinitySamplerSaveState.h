#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

class InfinitySamplerSaveState final
{

public:

  //Denotes whether or not a file or folder path has been saved.
  enum class SaveState : uint8
  {
    NONE,
    FILE,
    FOLDER
  } _SaveState{ SaveState::NONE };

  //The file path.
  char _FilePath[260];

  //The folder path.
  char _FolderPath[260];

};