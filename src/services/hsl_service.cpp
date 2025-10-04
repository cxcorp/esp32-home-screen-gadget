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
    static const char *hsl_entrust_tls_root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIE0DCCA7igAwIBAgIBBzANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"
        "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"
        "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"
        "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTExMDUwMzA3MDAwMFoXDTMxMDUwMzA3\n"
        "MDAwMFowgbQxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"
        "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjEtMCsGA1UE\n"
        "CxMkaHR0cDovL2NlcnRzLmdvZGFkZHkuY29tL3JlcG9zaXRvcnkvMTMwMQYDVQQD\n"
        "EypHbyBEYWRkeSBTZWN1cmUgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n"
        "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC54MsQ1K92vdSTYuswZLiBCGzD\n"
        "BNliF44v/z5lz4/OYuY8UhzaFkVLVat4a2ODYpDOD2lsmcgaFItMzEUz6ojcnqOv\n"
        "K/6AYZ15V8TPLvQ/MDxdR/yaFrzDN5ZBUY4RS1T4KL7QjL7wMDge87Am+GZHY23e\n"
        "cSZHjzhHU9FGHbTj3ADqRay9vHHZqm8A29vNMDp5T19MR/gd71vCxJ1gO7GyQ5HY\n"
        "pDNO6rPWJ0+tJYqlxvTV0KaudAVkV4i1RFXULSo6Pvi4vekyCgKUZMQWOlDxSq7n\n"
        "eTOvDCAHf+jfBDnCaQJsY1L6d8EbyHSHyLmTGFBUNUtpTrw700kuH9zB0lL7AgMB\n"
        "AAGjggEaMIIBFjAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNV\n"
        "HQ4EFgQUQMK9J47MNIMwojPX+2yz8LQsgM4wHwYDVR0jBBgwFoAUOpqFBxBnKLbv\n"
        "9r0FQW4gwZTaD94wNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
        "b2NzcC5nb2RhZGR5LmNvbS8wNQYDVR0fBC4wLDAqoCigJoYkaHR0cDovL2NybC5n\n"
        "b2RhZGR5LmNvbS9nZHJvb3QtZzIuY3JsMEYGA1UdIAQ/MD0wOwYEVR0gADAzMDEG\n"
        "CCsGAQUFBwIBFiVodHRwczovL2NlcnRzLmdvZGFkZHkuY29tL3JlcG9zaXRvcnkv\n"
        "MA0GCSqGSIb3DQEBCwUAA4IBAQAIfmyTEMg4uJapkEv/oV9PBO9sPpyIBslQj6Zz\n"
        "91cxG7685C/b+LrTW+C05+Z5Yg4MotdqY3MxtfWoSKQ7CC2iXZDXtHwlTxFWMMS2\n"
        "RJ17LJ3lXubvDGGqv+QqG+6EnriDfcFDzkSnE3ANkR/0yBOtg2DZ2HKocyQetawi\n"
        "DsoXiWJYRBuriSUBAA/NxBti21G00w9RKpv0vHP8ds42pM3Z2Czqrpv1KrKQ0U11\n"
        "GIo/ikGQI31bS/6kA1ibRrLDYGCD+H1QQc7CoZDDu+8CL9IVVO5EFdkKrqeKM+2x\n"
        "LXY2JtwE65/3YR8V3Idv7kaWKK2hJn0KCacuBKONvPi8BDAB\n"
        "-----END CERTIFICATE-----\n";

    static ArduinoJson::DynamicJsonDocument hslJson(4096);

    static bool fetchSchedule(JsonDocument &output)
    {
      WiFiClientSecure wifiClient;
      wifiClient.setCACert(hsl_entrust_tls_root_ca);

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