#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//Resources.
#include <Resources/Core/SoundResource.h>

class InfinityGuitarNote final
{

public:

  //The open notes.
  DynamicArray<SoundResource> _OpenNotes;

  //The muted notes.
  DynamicArray<SoundResource> _MutedNotes;

};