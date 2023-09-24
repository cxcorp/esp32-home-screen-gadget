#pragma once

#include <functional>

namespace NTP
{
  void setup();
  void block_until_synced(std::function<void()> waitFunction);
};