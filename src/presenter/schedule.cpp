#include "presenter.hpp"
#include <Arduino.h>
#include <lvgl.h>
#include <esp32_smartdisplay.h>
#include <ctime>
#include "ui/ui.h"
#include "time_utils.hpp"
#include "services/hsl_service.hpp"

#include "secrets.h"

namespace Presenter
{
  namespace Internal
  {
    static constexpr size_t scheduleItemCount = 5;
    // ui_main_screen.c creates the objects and assigns the pointers
    // to ui_schedule_*. grab pointer of those pointers so we don't
    // early bind to null pointers.
    struct UiSchedule
    {
      lv_obj_t **container;
      lv_obj_t **busnumberLabel;
      lv_obj_t **destinationLabel;
      lv_obj_t **timeLabel;
    };

    static UiSchedule uiSchedules[scheduleItemCount] = {
        {&ui_schedule1, &ui_schedule1_busnumber1, &ui_schedule1_destination1, &ui_schedule1_time1},
        {&ui_schedule2, &ui_schedule2_busnumber, &ui_schedule2_destination, &ui_schedule2_time},
        {&ui_schedule3, &ui_schedule3_busnumber, &ui_schedule3_destination, &ui_schedule3_time},
        {&ui_schedule4, &ui_schedule4_busnumber, &ui_schedule4_destination, &ui_schedule4_time},
        {&ui_schedule5, &ui_schedule5_busnumber, &ui_schedule5_destination, &ui_schedule5_time}};

    static void update_cancelled_schedule_item(UiSchedule &ui, const Services::HslService::StopTime &stoptime)
    {
      lv_obj_add_state(*ui.container, LV_STATE_DISABLED);
      lv_label_set_text(*ui.busnumberLabel, stoptime.routeName.c_str());
      lv_label_set_text(*ui.destinationLabel, "!Peruttu!");
      lv_label_set_text(*ui.timeLabel, LV_SYMBOL_MINUS);
      lv_obj_set_style_text_font(*ui.timeLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    static long get_time_required_to_reach_the_stop(const std::string &stopGtfsId)
    {
      if (stopGtfsId == HSL_STOP_ID)
      {
        return HSL_MINUTES_TO_WALK_TO_STOP * TimeUtils::MIN_TO_S;
      }
      return 0;
    }

    static bool can_make_it_to_stop_on_time(const std::string &stopGtfsId, long timeLeftToReachTheStop)
    {
      long timeRequiredToReachTheStop = get_time_required_to_reach_the_stop(stopGtfsId);
      return timeRequiredToReachTheStop < timeLeftToReachTheStop;
    }

    static void update_schedule_item(UiSchedule &ui, const Services::HslService::StopTime &stoptime, const time_t now)
    {
      lv_label_set_text(*ui.busnumberLabel, stoptime.routeName.c_str());
      lv_label_set_text(*ui.destinationLabel, stoptime.headsign.c_str());

      struct tm stopTimeinfo;
      localtime_r(&stoptime.timestamp, &stopTimeinfo);

      if (stoptime.timestamp > now && (stoptime.timestamp - now) >= 10 * TimeUtils::MIN_TO_S)
      {
        // If the upcoming bus is 10 or more minutes away, show HH:MM instead of
        // relative time
        std::string labelText = TimeUtils::to_format(&stopTimeinfo, "%H:%M");
        if (!stoptime.isRealtime)
        {
          labelText = "~" + labelText;
        }
        lv_label_set_text(*ui.timeLabel, labelText.c_str());
        lv_obj_set_style_text_font(*ui.timeLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

        // more than 10 minutes away -> you can still make it to this bus
        lv_obj_clear_state(*ui.container, LV_STATE_DISABLED);
      }
      else
      {
        long secondsUntilScheduledArrival = stoptime.timestamp - now;
        long minutesUntilScheduledArrival = secondsUntilScheduledArrival / 60L;
        std::string labelText = std::to_string(minutesUntilScheduledArrival);
        if (!stoptime.isRealtime)
        {
          labelText = "~" + labelText;
        }

        lv_label_set_text(*ui.timeLabel, labelText.c_str());
        lv_obj_set_style_text_font(*ui.timeLabel, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (can_make_it_to_stop_on_time(stoptime.stopGtfsId, secondsUntilScheduledArrival))
        {
          lv_obj_clear_state(*ui.container, LV_STATE_DISABLED);
        }
        else
        {
          lv_obj_add_state(*ui.container, LV_STATE_DISABLED);
        }
      }
    }

    void present_schedule(const UiModel::Schedule &schedule)
    {
      if (!schedule.hasChanged())
      {
        return;
      }

      std::vector<Services::HslService::StopTime> stopTimesThatHaveNotPassedAlready;
      time_t now = time(nullptr);

      {
        // filter only those schedule items which haven't passed already
        const std::vector<Services::HslService::StopTime> *stopTimes = &schedule.getStopTimes();
        std::copy_if(
            stopTimes->begin(),
            stopTimes->end(),
            std::back_inserter(stopTimesThatHaveNotPassedAlready),
            [now](const Services::HslService::StopTime &stoptime) -> bool
            {
              return now <= stoptime.timestamp;
            });
      }

      if (stopTimesThatHaveNotPassedAlready.size() < scheduleItemCount)
      {
        // If there's fewer schedule items available than what we can show
        // in the UI, hide the rest of the schedule UI elements
        {
          const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);

          for (size_t i = stopTimesThatHaveNotPassedAlready.size(); i < scheduleItemCount; ++i)
          {
            lv_obj_add_flag(*uiSchedules[i].container, LV_OBJ_FLAG_HIDDEN);
          }
        }
      }

      for (size_t i = 0; i < stopTimesThatHaveNotPassedAlready.size() && i < scheduleItemCount; ++i)
      {
        const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
        auto stoptime = stopTimesThatHaveNotPassedAlready[i];
        auto ui = uiSchedules[i];

        // unhide the element in case it was previously hidden
        lv_obj_clear_flag(*ui.container, LV_OBJ_FLAG_HIDDEN);

        if (stoptime.isCancelled)
        {
          update_cancelled_schedule_item(ui, stoptime);
        }
        else
        {
          update_schedule_item(ui, stoptime, now);
        }
      }
    }
  };
};