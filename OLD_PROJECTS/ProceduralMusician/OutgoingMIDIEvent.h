#pragma once

//Core.
#include <Core/Essential/CatalystEssential.h>

class OutgoingMIDIEvent final
{

public:

  //The message.
  iplug::IMidiMsg _Message;

  //The offset.
  int32 _Offset;

  /*
  * Constructor taking all values as arguments.
  */
  FORCE_INLINE OutgoingMIDIEvent(const iplug::IMidiMsg &message, const int32 offset) NOEXCEPT
    :
    _Message(message),
    _Offset(offset)
  {

  }

};