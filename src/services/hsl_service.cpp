#include "services/hsl_service.hpp"
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <cstring>
#include <algorithm>

#include "secrets.h"

namespace Services
{
  namespace HslService
  {
    static const char *hsl_graphql_query =
        "{\n"
        "  stops(ids: [\"" HSL_STOP_ID "\"]) {\n"
        "    gtfsId\n"
        "    stoptimesWithoutPatterns(\n"
        "      numberOfDepartures: 3,\n"
        "      omitCanceled: false\n"
        "    ) {\n"
        "      realtimeArrival\n"
        "      realtime\n"
        "      realtimeState\n"
        "      serviceDay\n"
        "      trip {\n"
        "        routeShortName\n"
        "        tripHeadsign\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";

    // get the root cert with `openssl s_client -showcerts -connect api.digitransit.fi:443`
    static const char *hsl_google_gts_root_r4_tls_root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
        "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
        "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
        "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
        "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
        "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
        "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
        "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
        "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
        "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
        "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
        "-----END CERTIFICATE-----\n";

    static ArduinoJson::JsonDocument hslJson;

    static bool fetchSchedule(JsonDocument &output)
    {
      WiFiClientSecure wifiClient;
      wifiClient.setCACert(hsl_google_gts_root_r4_tls_root_ca);

      {
        HTTPClient https;
        https.useHTTP10(true);

        const char *url = "https://api.digitransit.fi/routing/v2/hsl/gtfs/v1";
        Serial.printf("[HslService::fetchSchedule] fetching: \"%s\"\n", url);
        https.begin(wifiClient, url);
        https.addHeader("digitransit-subscription-key", DIGITRANSIT_SUBSCRIPTION_KEY);
        https.addHeader("Content-Type", "application/graphql");
        https.addHeader("Accept", "application/json; charset=UTF-8");

        int httpCode = https.POST(hsl_graphql_query);
        Serial.printf("[HslService::fetchSchedule] returned http %d\n", httpCode);
        if (httpCode != HTTP_CODE_OK)
        {
          Serial.println(https.getString());
          https.end();
          return false;
        }

        DeserializationError error = deserializeJson(output, https.getStream());
        if (error)
        {
          Serial.printf("[HslService::fetchSchedule] JSON deserialization failed: ");
          Serial.println(error.c_str());
          https.end();
          return false;
        }

        https.end();
      }

      return true;
    }

    bool getSchedule(std::vector<StopTime> &output)
    {
      if (!fetchSchedule(hslJson))
      {
        Serial.println("[HslService::getSchedule] fetchSchedule failed -> abort");
        return false;
      }

      for (JsonObject stop : hslJson["data"]["stops"].as<JsonArray>())
      {
        const char *stopGtfsId = stop["gtfsId"];

        for (JsonObject stopTime : stop["stoptimesWithoutPatterns"].as<JsonArray>())
        {
          signed long arrivalTimeOffset = stopTime["realtimeArrival"]; // may be negative but not larger than serviceDay
          bool isRealtime = stopTime["realtime"];
          const char *realtimeState = stopTime["realtimeState"];
          signed long serviceDay = stopTime["serviceDay"];
          const char *routeShortName = stopTime["trip"]["routeShortName"];
          const char *headsign = stopTime["trip"]["tripHeadsign"];

          time_t timestamp = (time_t)(serviceDay + arrivalTimeOffset);

          output.emplace_back(
              StopTime{
                  .stopGtfsId = stopGtfsId,
                  .timestamp = timestamp,
                  .isRealtime = isRealtime,
                  .isCancelled = strcmp(realtimeState, "CANCELLED") == 0,
                  .routeName = routeShortName,
                  .headsign = headsign});
        }
      }

      std::sort(
          output.begin(),
          output.end(),
          [](const StopTime &a, const StopTime &b)
          {
            return a.timestamp < b.timestamp;
          });

      return true;
    }
  };
};