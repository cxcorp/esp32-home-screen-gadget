#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace Services
{
  namespace HslService
  {
    struct StopTime
    {
      std::string stopGtfsId;
      time_t timestamp;
      bool isRealtime;
      bool isCancelled;
      std::string routeName;
      std::string headsign;
    };

    bool getSchedule(std::vector<StopTime> &output);
  };
};