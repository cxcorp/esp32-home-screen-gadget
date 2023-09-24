#pragma once
#include <ctime>
#include <string>

namespace TimeUtils
{
  constexpr unsigned long S_TO_MS = 1000;
  constexpr unsigned long MIN_TO_S = 60;
  constexpr unsigned long HOUR_TO_S = 60 * MIN_TO_S;
  constexpr unsigned long THREE_HOURS_TO_S = 3 * HOUR_TO_S;

  const struct tm *get_localtime();

  void get_localtime_r(struct tm *);

  std::string to_format(const tm *timeinfo, const char *format);

  std::string to_iso8601(const tm *timeinfo);

  std::string to_date_display_format(const tm *timeinfo);

  std::string to_time_display_format(const tm *timeinfo);
}