#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//Resources.
#include <Resources/Core/SoundResource.h>

class InfinityBassNote final
{

public:

  //The open notes.
  DynamicArray<SoundResource> _OpenNotes;

  //The dead notes.
  DynamicArray<SoundResource> _MutedNotes;

};