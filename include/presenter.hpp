#pragma once
#include "ui_model/screen_model.hpp"

namespace Presenter
{
  void present_screen(const UiModel::Screen &screen);

  namespace Internal
  {
    void present_clock(const UiModel::Clock &weather);
    void present_weather(const UiModel::Weather &weather);
    void present_schedule(const UiModel::Schedule &schedule);
  }
};
