#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>
#include <Core/Containers/DynamicArray.h>

//Procedural musician.
#include "OnMIDIEventContext.h"

//Sound.
#include <Sound/SoundUtilities.h>

class SectionGenerator
{

public:

  /*
  * Default constructor.
  */
  FORCE_INLINE SectionGenerator() NOEXCEPT
  {

  }

  /*
  * Default destructor.
  */
  FORCE_INLINE virtual ~SectionGenerator() NOEXCEPT
  {

  }

  /*
  * The on MIDI event function.
  */
  FORCE_INLINE virtual void OnMIDIEvent(const OnMIDIEventContext &context) NOEXCEPT
  {

  }

};