#include "presenter.hpp"
#include <esp32_smartdisplay.h>
#include <sstream>
#include <iomanip>
#include "ui/ui.h"
#include "time_utils.hpp"

namespace Presenter
{
  namespace Internal
  {
    static const char *localizedWeekdays[7] = {
        "sunnuntai",
        "maanantai",
        "tiistai",
        "keskiviikko",
        "torstai",
        "perjantai",
        "lauantai"};

    static void render_date(const struct tm *timeinfo)
    {
      std::ostringstream os;
      os << localizedWeekdays[timeinfo->tm_wday] << "\n";
      // would otherwise use std::put_time but there is no month
      // format without a leading zero
      os << timeinfo->tm_mday << "." << (timeinfo->tm_mon + 1) << ".";

      std::string dateString = os.str();
      // lv_label_set_text makes a copy of the string

      {
        const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
        lv_label_set_text(ui_date_label, dateString.c_str());
      }
    }

    static void render_time(const struct tm *timeinfo)
    {
      std::string timeString = TimeUtils::to_format(timeinfo, "%H:%M");
      {
        const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
        lv_label_set_text(ui_time_label, timeString.c_str());
      }
    }

    void present_clock(const UiModel::Clock &clock)
    {
      if (!clock.hasChanged())
      {
        return;
      }

      time_t rawTime = clock.getTime();
      const struct tm *timeinfo = localtime(&rawTime);

      render_date(timeinfo);
      render_time(timeinfo);
    }
  };
};