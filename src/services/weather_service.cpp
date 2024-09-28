#include "services/weather_service.hpp"

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "string_utils.hpp"
#include "time_utils.hpp"

#include "secrets.h"

namespace Services
{
  namespace WeatherService
  {
    // get the root cert with `openssl s_client -showcerts -connect opendata.fmi.fi:443`
    static const char *opendata_fmi_fi_tls_root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIFgTCCBGmgAwIBAgIQOXJEOvkit1HX02wQ3TE1lTANBgkqhkiG9w0BAQwFADB7\n"
        "MQswCQYDVQQGEwJHQjEbMBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYD\n"
        "VQQHDAdTYWxmb3JkMRowGAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UE\n"
        "AwwYQUFBIENlcnRpZmljYXRlIFNlcnZpY2VzMB4XDTE5MDMxMjAwMDAwMFoXDTI4\n"
        "MTIzMTIzNTk1OVowgYgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpOZXcgSmVyc2V5\n"
        "MRQwEgYDVQQHEwtKZXJzZXkgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBO\n"
        "ZXR3b3JrMS4wLAYDVQQDEyVVU0VSVHJ1c3QgUlNBIENlcnRpZmljYXRpb24gQXV0\n"
        "aG9yaXR5MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAgBJlFzYOw9sI\n"
        "s9CsVw127c0n00ytUINh4qogTQktZAnczomfzD2p7PbPwdzx07HWezcoEStH2jnG\n"
        "vDoZtF+mvX2do2NCtnbyqTsrkfjib9DsFiCQCT7i6HTJGLSR1GJk23+jBvGIGGqQ\n"
        "Ijy8/hPwhxR79uQfjtTkUcYRZ0YIUcuGFFQ/vDP+fmyc/xadGL1RjjWmp2bIcmfb\n"
        "IWax1Jt4A8BQOujM8Ny8nkz+rwWWNR9XWrf/zvk9tyy29lTdyOcSOk2uTIq3XJq0\n"
        "tyA9yn8iNK5+O2hmAUTnAU5GU5szYPeUvlM3kHND8zLDU+/bqv50TmnHa4xgk97E\n"
        "xwzf4TKuzJM7UXiVZ4vuPVb+DNBpDxsP8yUmazNt925H+nND5X4OpWaxKXwyhGNV\n"
        "icQNwZNUMBkTrNN9N6frXTpsNVzbQdcS2qlJC9/YgIoJk2KOtWbPJYjNhLixP6Q5\n"
        "D9kCnusSTJV882sFqV4Wg8y4Z+LoE53MW4LTTLPtW//e5XOsIzstAL81VXQJSdhJ\n"
        "WBp/kjbmUZIO8yZ9HE0XvMnsQybQv0FfQKlERPSZ51eHnlAfV1SoPv10Yy+xUGUJ\n"
        "5lhCLkMaTLTwJUdZ+gQek9QmRkpQgbLevni3/GcV4clXhB4PY9bpYrrWX1Uu6lzG\n"
        "KAgEJTm4Diup8kyXHAc/DVL17e8vgg8CAwEAAaOB8jCB7zAfBgNVHSMEGDAWgBSg\n"
        "EQojPpbxB+zirynvgqV/0DCktDAdBgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rID\n"
        "ZsswDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAG\n"
        "BgRVHSAAMEMGA1UdHwQ8MDowOKA2oDSGMmh0dHA6Ly9jcmwuY29tb2RvY2EuY29t\n"
        "L0FBQUNlcnRpZmljYXRlU2VydmljZXMuY3JsMDQGCCsGAQUFBwEBBCgwJjAkBggr\n"
        "BgEFBQcwAYYYaHR0cDovL29jc3AuY29tb2RvY2EuY29tMA0GCSqGSIb3DQEBDAUA\n"
        "A4IBAQAYh1HcdCE9nIrgJ7cz0C7M7PDmy14R3iJvm3WOnnL+5Nb+qh+cli3vA0p+\n"
        "rvSNb3I8QzvAP+u431yqqcau8vzY7qN7Q/aGNnwU4M309z/+3ri0ivCRlv79Q2R+\n"
        "/czSAaF9ffgZGclCKxO/WIu6pKJmBHaIkU4MiRTOok3JMrO66BQavHHxW/BBC5gA\n"
        "CiIDEOUMsfnNkjcZ7Tvx5Dq2+UUTJnWvu6rvP3t3O9LEApE9GQDTF1w52z97GA1F\n"
        "zZOFli9d31kWTz9RvdVFGD/tSo7oBmF0Ixa1DVBzJ0RHfxBdiSprhTEUxOipakyA\n"
        "vGp4z7h/jnZymQyd/teRCBaho1+V\n"
        "-----END CERTIFICATE-----\n";

    static ArduinoJson::DynamicJsonDocument weatherJson(2048);

    static std::string buildWeatherApiUrl()
    {
      struct tm timeinfoStart;
      struct tm timeinfoEnd;

      {
        time_t start = time(nullptr);
        // round time down to nearest 3 hour interval
        start -= (start % TimeUtils::HOUR_TO_S);
        gmtime_r(&start, &timeinfoStart);

        time_t end = start;
        // get enough results for the next five 3 hour increments
        // but ask for one more just in case
        end += TimeUtils::THREE_HOURS_TO_S * 6;
        gmtime_r(&end, &timeinfoEnd);
      }

      std::ostringstream of;
      of << "https://opendata.fmi.fi/timeseries"
         << "?latlon=" << StringUtils::percentEncode(WEATHER_COORDINATES_LATLON)
         << "&param=" << StringUtils::percentEncode("epochtime,temperature,smartsymbol")
         << "&timestep=" << (3 * 60)
         << "&tz=" << StringUtils::percentEncode("Europe/Helsinki")
         << "&format=json"
         << "&starttime=" << StringUtils::percentEncode(TimeUtils::to_iso8601(&timeinfoStart))
         << "&endtime=" << StringUtils::percentEncode(TimeUtils::to_iso8601(&timeinfoEnd));
      return of.str();
    }

    static bool fetchWeather(JsonDocument &output)
    {
      WiFiClientSecure wifiClient;
      wifiClient.setCACert(opendata_fmi_fi_tls_root_ca);

      {
        // scope http so it's deconstructed before wifiClient
        std::string url = buildWeatherApiUrl();
        HTTPClient https;
        // ArduinJSON doesn't work with chunked encoding so use HTTP/1.0
        https.useHTTP10(true);
        Serial.printf("[WeatherService::fetchWeather] Fetching: \"%s\"\n", url.c_str());
        https.begin(wifiClient, String(url.c_str()));
        https.addHeader("Accept", "application/json; charset=UTF-8");

        int httpCode = https.GET();
        Serial.printf("[WeatherService::fetchWeather] returned http %d\n", httpCode);
        if (httpCode != HTTP_CODE_OK)
        {
          Serial.println(https.getString());
          https.end();
          return false;
        }

        DeserializationError error = deserializeJson(output, https.getStream());
        if (error)
        {
          Serial.printf("[WeatherService::fetchWeather] JSON deserialization failed: ");
          Serial.println(error.c_str());
          https.end();
          return false;
        }

        https.end();
      }

      return true;
    }

    bool getWeatherForecasts(std::vector<WeatherForecast> &output)
    {
      if (!fetchWeather(weatherJson))
      {
        return false;
      }

      Serial.println("[WeatherService::getWeatherForecasts] Got forecasts:");
      size_t i = 0;
      for (JsonObject item : weatherJson.as<JsonArray>())
      {
        unsigned long epochtime = item["epochtime"];     // 1695448800, 1695459600, 1695470400, 1695481200, 1695492000, ...
        unsigned long temperature = item["temperature"]; // 17, 17, 16, 16, 16, 15
        unsigned long smartsymbol = item["smartsymbol"]; // 6, 38, 37, 27, 104, 104
        Serial.printf("%lu\t%lu\t%lu.png\n", epochtime, temperature, smartsymbol);
        output.emplace_back(
            WeatherForecast{
                .timestamp = (time_t)epochtime,
                .temperatureCelsius = temperature,
                .smartSymbolId = smartsymbol});

        if (++i > 5)
        {
          break;
        }
      }

      return true;
    }
  };
};
