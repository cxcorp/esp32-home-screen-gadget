#include "device/ntp.hpp"
#include <Arduino.h>
#include <esp_sntp.h>
#include "time_utils.hpp"

namespace NTP
{
  void setup()
  {
    // TZ string for Europe/Helsinki, updated 2023-09-21
    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    const char *tzString = "EET-2EEST,M3.5.0/3,M10.5.0/4";
    configTzTime(
        tzString,
        "0.fi.pool.ntp.org",
        "1.fi.pool.ntp.org",
        "2.fi.pool.ntp.org");
  }

  void block_until_synced(std::function<void()> waitFunction)
  {
    Serial.println("Waiting for NTP time sync...");
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
      waitFunction();
    }
    Serial.println("\nCompleted!");

    std::string time = TimeUtils::to_iso8601(TimeUtils::get_localtime());
    Serial.printf("Current time: %s\n", time.c_str());
  }
}