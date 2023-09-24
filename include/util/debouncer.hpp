#pragma once

namespace Util
{
  class Debouncer
  {
  public:
    Debouncer(unsigned long debounceMilliseconds);
    bool tryElapse();

  private:
    const unsigned long debounceMilliseconds;
    unsigned long prevUpdate;
  };
};