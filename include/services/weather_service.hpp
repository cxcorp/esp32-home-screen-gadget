#pragma once

#include <vector>
#include <ctime>

namespace Services
{
  namespace WeatherService
  {
    struct WeatherForecast
    {
      const time_t timestamp;
      const long temperatureCelsius;
      const unsigned long smartSymbolId;
    };

    bool getWeatherForecasts(std::vector<WeatherForecast> &output);
  };
};
