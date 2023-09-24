#include "presenter.hpp"

namespace Presenter
{
  void present_screen(const UiModel::Screen &screen)
  {
    if (!screen.hasChanged())
    {
      return;
    }

    Internal::present_clock(screen.getClock());
    Internal::present_weather(screen.getWeather());
    Internal::present_schedule(screen.getSchedule());
  }
};