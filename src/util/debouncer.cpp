#include "util/debouncer.hpp"
#include <Arduino.h>

namespace Util
{
  Debouncer::Debouncer(unsigned long debounceMs)
      : debounceMilliseconds(debounceMs), prevUpdate(0) {}

  bool Debouncer::tryElapse()
  {
    unsigned long now = millis();
    if (this->prevUpdate == 0 || (now - this->prevUpdate > this->debounceMilliseconds))
    {
      this->prevUpdate = now;
      return true;
    }
    return false;
  }
};