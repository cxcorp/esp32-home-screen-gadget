#include "time_utils.hpp"
#include <sstream>
#include <iomanip>

namespace TimeUtils
{

  const struct tm *get_localtime()
  {
    time_t raw_time;
    time(&raw_time);
    return localtime(&raw_time);
  }

  void get_localtime_r(struct tm *timeinfo)
  {
    time_t raw_time;
    time(&raw_time);
    localtime_r(&raw_time, timeinfo);
  }

  std::string to_format(const tm *timeinfo, const char *format)
  {
    std::ostringstream os;
    os << std::put_time(timeinfo, format);
    return os.str();
  }

  std::string to_iso8601(const tm *timeinfo)
  {
    return to_format(timeinfo, "%FT%T%z");
  }

  std::string to_date_display_format(const tm *timeinfo)
  {
    return to_format(timeinfo, "%A %d.%m.%Y");
  }

  std::string to_time_display_format(const tm *timeinfo)
  {
    return to_format(timeinfo, "%H:%M");
  }
}