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
        "  stops(ids: [\"HSL:1203402\", \"HSL:1203425\"]) {\n"
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

    static const char *hsl_entrust_tls_root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIEPjCCAyagAwIBAgIESlOMKDANBgkqhkiG9w0BAQsFADCBvjELMAkGA1UEBhMC\n"
        "VVMxFjAUBgNVBAoTDUVudHJ1c3QsIEluYy4xKDAmBgNVBAsTH1NlZSB3d3cuZW50\n"
        "cnVzdC5uZXQvbGVnYWwtdGVybXMxOTA3BgNVBAsTMChjKSAyMDA5IEVudHJ1c3Qs\n"
        "IEluYy4gLSBmb3IgYXV0aG9yaXplZCB1c2Ugb25seTEyMDAGA1UEAxMpRW50cnVz\n"
        "dCBSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5IC0gRzIwHhcNMDkwNzA3MTcy\n"
        "NTU0WhcNMzAxMjA3MTc1NTU0WjCBvjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUVu\n"
        "dHJ1c3QsIEluYy4xKDAmBgNVBAsTH1NlZSB3d3cuZW50cnVzdC5uZXQvbGVnYWwt\n"
        "dGVybXMxOTA3BgNVBAsTMChjKSAyMDA5IEVudHJ1c3QsIEluYy4gLSBmb3IgYXV0\n"
        "aG9yaXplZCB1c2Ugb25seTEyMDAGA1UEAxMpRW50cnVzdCBSb290IENlcnRpZmlj\n"
        "YXRpb24gQXV0aG9yaXR5IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
        "AoIBAQC6hLZy254Ma+KZ6TABp3bqMriVQRrJ2mFOWHLP/vaCeb9zYQYKpSfYs1/T\n"
        "RU4cctZOMvJyig/3gxnQaoCAAEUesMfnmr8SVycco2gvCoe9amsOXmXzHHfV1IWN\n"
        "cCG0szLni6LVhjkCsbjSR87kyUnEO6fe+1R9V77w6G7CebI6C1XiUJgWMhNcL3hW\n"
        "wcKUs/Ja5CeanyTXxuzQmyWC48zCxEXFjJd6BmsqEZ+pCm5IO2/b1BEZQvePB7/1\n"
        "U1+cPvQXLOZprE4yTGJ36rfo5bs0vBmLrpxR57d+tVOxMyLlbc9wPBr64ptntoP0\n"
        "jaWvYkxN4FisZDQSA/i2jZRjJKRxAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAP\n"
        "BgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBRqciZ60B7vfec7aVHUbI2fkBJmqzAN\n"
        "BgkqhkiG9w0BAQsFAAOCAQEAeZ8dlsa2eT8ijYfThwMEYGprmi5ZiXMRrEPR9RP/\n"
        "jTkrwPK9T3CMqS/qF8QLVJ7UG5aYMzyorWKiAHarWWluBh1+xLlEjZivEtRh2woZ\n"
        "Rkfz6/djwUAFQKXSt/S1mja/qYh2iARVBCuch38aNzx+LaUa2NSJXsq9rD1s2G2v\n"
        "1fN2D807iDginWyTmsQ9v4IbZT+mD12q/OWyFcq1rca8PdCE6OoGcrBNOTJ4vz4R\n"
        "nAuknZoh8/CbCzB428Hch0P+vGOaysXCHMnHjf87ElgI5rY97HosTvuDls4MPGmH\n"
        "VHOkc8KT/1EQrBVUAdj8BbGJoX90g5pJ19xOe4pIb4tF9g==\n"
        "-----END CERTIFICATE-----\n";

    static ArduinoJson::StaticJsonDocument<4096> hslJson;

    static bool fetchSchedule(JsonDocument &output)
    {
      WiFiClientSecure wifiClient;
      wifiClient.setCACert(hsl_entrust_tls_root_ca);

      {
        HTTPClient https;
        https.useHTTP10(true);

        const char *url = "https://api.digitransit.fi/routing/v1/routers/hsl/index/graphql";
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