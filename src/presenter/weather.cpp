#include "presenter.hpp"
#include <Arduino.h>
#include <lvgl.h>
#include <esp32_smartdisplay.h>
#include <map>
#include "ui/ui.h"
#include "time_utils.hpp"
#include "services/weather_service.hpp"

namespace Presenter
{
  namespace Internal
  {

    static std::map<unsigned long, const lv_img_dsc_t *> smartSymbols = {{1, &icon_1},
                                                                         {2, &icon_2},
                                                                         {4, &icon_4},
                                                                         {6, &icon_6},
                                                                         {7, &icon_7},
                                                                         {9, &icon_9},
                                                                         {11, &icon_11},
                                                                         {14, &icon_14},
                                                                         {17, &icon_17},
                                                                         {21, &icon_21},
                                                                         {24, &icon_24},
                                                                         {27, &icon_27},
                                                                         {31, &icon_31},
                                                                         {32, &icon_32},
                                                                         {33, &icon_33},
                                                                         {34, &icon_34},
                                                                         {35, &icon_35},
                                                                         {36, &icon_36},
                                                                         {37, &icon_37},
                                                                         {38, &icon_38},
                                                                         {39, &icon_39},
                                                                         {41, &icon_41},
                                                                         {42, &icon_42},
                                                                         {43, &icon_43},
                                                                         {44, &icon_44},
                                                                         {45, &icon_45},
                                                                         {46, &icon_46},
                                                                         {47, &icon_47},
                                                                         {48, &icon_48},
                                                                         {49, &icon_49},
                                                                         {51, &icon_51},
                                                                         {52, &icon_52},
                                                                         {53, &icon_53},
                                                                         {54, &icon_54},
                                                                         {55, &icon_55},
                                                                         {56, &icon_56},
                                                                         {57, &icon_57},
                                                                         {58, &icon_58},
                                                                         {59, &icon_59},
                                                                         {61, &icon_61},
                                                                         {64, &icon_64},
                                                                         {67, &icon_67},
                                                                         {71, &icon_71},
                                                                         {74, &icon_74},
                                                                         {77, &icon_77},
                                                                         {101, &icon_101},
                                                                         {102, &icon_102},
                                                                         {104, &icon_104},
                                                                         {106, &icon_106},
                                                                         {121, &icon_121},
                                                                         {124, &icon_124},
                                                                         {131, &icon_131},
                                                                         {132, &icon_132},
                                                                         {133, &icon_133},
                                                                         {134, &icon_134},
                                                                         {135, &icon_135},
                                                                         {136, &icon_136},
                                                                         {141, &icon_141},
                                                                         {142, &icon_142},
                                                                         {143, &icon_143},
                                                                         {144, &icon_144},
                                                                         {145, &icon_145},
                                                                         {146, &icon_146},
                                                                         {151, &icon_151},
                                                                         {152, &icon_152},
                                                                         {153, &icon_153},
                                                                         {154, &icon_154},
                                                                         {155, &icon_155},
                                                                         {156, &icon_156},
                                                                         {161, &icon_161},
                                                                         {164, &icon_164},
                                                                         {171, &icon_171},
                                                                         {174, &icon_174}};

    static constexpr size_t weatherItemCount = 5;
    // ui_main_screen.c creates the objects and assigns the pointers
    // to ui_weather_*. grab pointer of those pointers so we don't
    // early bind to null pointers.
    static lv_obj_t **weatherContainers[weatherItemCount] = {
        &ui_weather1,
        &ui_weather2,
        &ui_weather3,
        &ui_weather4,
        &ui_weather5};
    static lv_obj_t **weatherHourLabels[weatherItemCount] = {
        &ui_weather1_hour,
        &ui_weather2_hour,
        &ui_weather3_hour,
        &ui_weather4_hour,
        &ui_weather5_hour};
    static lv_obj_t **weatherImages[weatherItemCount] = {
        &ui_weather1_icon,
        &ui_weather2_icon,
        &ui_weather3_icon,
        &ui_weather4_icon,
        &ui_weather5_icon};
    static lv_obj_t **weatherTemperatureLabels[weatherItemCount] = {
        &ui_weather1_temperature,
        &ui_weather2_temperature,
        &ui_weather3_temperature,
        &ui_weather4_temperature,
        &ui_weather5_temperature};

    static const lv_img_dsc_t *find_smart_symbol(const unsigned long smartSymbolId)
    {
      auto it = smartSymbols.find(smartSymbolId);
      if (it != smartSymbols.end())
      {
        return it->second;
      }

      // If it's a nighttime variant, it should be >100
      if (smartSymbolId > 100)
      {
        it = smartSymbols.find(smartSymbolId - 100);
        if (it != smartSymbols.end())
        {
          return it->second;
        }
      }

      // nope, we don't have it
      return nullptr;
    }

    static void update_weather_icon_image(lv_obj_t *imageControl, const lv_img_dsc_t *icon)
    {
      if (icon == nullptr)
      {
        lv_obj_add_flag(imageControl, LV_OBJ_FLAG_HIDDEN);
        return;
      }

      lv_img_set_src(imageControl, icon);
      lv_obj_clear_flag(imageControl, LV_OBJ_FLAG_HIDDEN);
    }

    void present_weather(const UiModel::Weather &weather)
    {
      if (!weather.hasChanged())
      {
        return;
      }

      const std::vector<Services::WeatherService::WeatherForecast> *forecasts = &weather.getForecasts();

      // update item content
      for (size_t i = 0; i < forecasts->size() && i < weatherItemCount; ++i)
      {
        auto forecast = &forecasts->at(i);

        struct tm timeinfo;
        localtime_r(&forecast->timestamp, &timeinfo);
        std::string hour = TimeUtils::to_format(&timeinfo, "%H");

        const lv_img_dsc_t *icon = find_smart_symbol(forecast->smartSymbolId);

        std::string temperature = std::to_string(forecast->temperatureCelsius);
        temperature += "°";

        {
          const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
          // Update hour label
          lv_label_set_text(*weatherHourLabels[i], hour.c_str());
          // Update icon
          update_weather_icon_image(*weatherImages[i], icon);
          // Update temperature
          lv_label_set_text(*weatherTemperatureLabels[i], temperature.c_str());
        }
      }

      // update "day changed" markers
      if (forecasts->size() < 2)
      {
        // we should always get at least 5 but let's be sure
        return;
      }

      for (size_t i = 1; i < forecasts->size() && i < weatherItemCount; ++i)
      {
        auto current = &forecasts->at(i);
        auto previous = &forecasts->at(i - 1);

        struct tm timeinfoCurrent, timeinfoPrevious;
        localtime_r(&current->timestamp, &timeinfoCurrent);
        localtime_r(&previous->timestamp, &timeinfoPrevious);

        {
          const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
          if (timeinfoPrevious.tm_yday != timeinfoCurrent.tm_yday)
          {
            // We've associated the "day changed" style with the USER_1 state
            lv_obj_add_state(*weatherContainers[i], LV_STATE_USER_1);
          }
          else
          {
            lv_obj_clear_state(*weatherContainers[i], LV_STATE_USER_1);
          }
        }
      }
    }
  };
};