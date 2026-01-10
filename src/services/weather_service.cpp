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
        "MIIGwDCCBKigAwIBAgIQKmCG1NTeRcleS5j7vy+/JjANBgkqhkiG9w0BAQsFADCB\n"
        "pjELMAkGA1UEBhMCR1IxDzANBgNVBAcTBkF0aGVuczFEMEIGA1UEChM7SGVsbGVu\n"
        "aWMgQWNhZGVtaWMgYW5kIFJlc2VhcmNoIEluc3RpdHV0aW9ucyBDZXJ0LiBBdXRo\n"
        "b3JpdHkxQDA+BgNVBAMTN0hlbGxlbmljIEFjYWRlbWljIGFuZCBSZXNlYXJjaCBJ\n"
        "bnN0aXR1dGlvbnMgUm9vdENBIDIwMTUwHhcNMjEwOTAyMDc0MTU1WhcNMjkwODMx\n"
        "MDc0MTU0WjBsMQswCQYDVQQGEwJHUjE3MDUGA1UECgwuSGVsbGVuaWMgQWNhZGVt\n"
        "aWMgYW5kIFJlc2VhcmNoIEluc3RpdHV0aW9ucyBDQTEkMCIGA1UEAwwbSEFSSUNB\n"
        "IFRMUyBSU0EgUm9vdCBDQSAyMDIxMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIIC\n"
        "CgKCAgEAi8Lnr2WbBWeWyQ0kudAOZPzO4iQYLIR/d1HLBBE2uF7taXGnnuQlCZdn\n"
        "wUfCz5EWNmI9OAThUYL/rNK0ad0u7BGjRe5raztMv4yNpB6dEbnpOPl6DgyY4iMd\n"
        "0U5j1Oe4QUT7a69r2h/TxZGIW6SJktGB5ow5WKDWaUOprZhSWG7bCvtrz2j646Re\n"
        "OkVzmAfqXwJy3gyls5+uqR23HbP8ilnnbnJlrfUwlCMH84IWSzWYnFO7L8rkWtnH\n"
        "jR38mJn7LKSCa/AqH44LX3FcXK5CeymJgcsDo5nKiJ4LQAlBM9vmWHr9rplwwFoP\n"
        "1hOGcS92afyQ3dstbtHym/Uaa55vFYx68EsooCI4gCRsNqQ78jCR83gTz8E/Navx\n"
        "HREjtUMingGStxgC5RHRgtsVAMxhN8EqfJrh0LqzUEbugqydMfj7I+IDAEhwowkm\n"
        "eRVTYPM4XK046oEAYxS5M17dC9ugRQcaMwn4TbSnAqZp9MJZBYhlhVauS8vg3jx9\n"
        "LRrI6fsfo2FK1ioTrXdMGhibkQ9Y2AZUxZf4qj8giqaFpnf2pvwc4u5ulDMqg1CE\n"
        "CuVPhvhQRXgAgetbaOMmjcx7XFH0FCxAvhpgHXpyYR0fYy2Iqs6iRZAI/Gu+s1Aq\n"
        "Wv2oSBhG1pBAkpAKhF5oMfjr7Q3THcZ9mRhVVidlLo1FxSTszuMCAwEAAaOCASEw\n"
        "ggEdMA8GA1UdEwEB/wQFMAMBAf8wHwYDVR0jBBgwFoAUcRVnyMjJvXVdctA4GGqd\n"
        "83EkVAswTAYIKwYBBQUHAQEEQDA+MDwGCCsGAQUFBzAChjBodHRwOi8vcmVwby5o\n"
        "YXJpY2EuZ3IvY2VydHMvSGFyaWNhUm9vdENBMjAxNS5jcnQwEQYDVR0gBAowCDAG\n"
        "BgRVHSAAMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATA6BgNVHR8EMzAx\n"
        "MC+gLaArhilodHRwOi8vY3JsLmhhcmljYS5nci9IYXJpY2FSb290Q0EyMDE1LmNy\n"
        "bDAdBgNVHQ4EFgQUCkgjpmCkkgoz6pNbxVfqJU29Eu4wDgYDVR0PAQH/BAQDAgGG\n"
        "MA0GCSqGSIb3DQEBCwUAA4ICAQDAze/rVni3cYGP1BWkxXnIVynqHcC6Cw7kbkXo\n"
        "XBZAqF9ANQAKoz2YQ+WyzrparKVilMG6p6mdWTKeLy6eQDRhFSu9DrMYr4TBYxIp\n"
        "2e5ZLQBk96xbG3wSepBCkTDA0MA0NybzM0gF5nTa82BMCED8EQXliWWsgqUorXuk\n"
        "7msadosX793M42llmAcj1zJkADzlyzi4gcPvIc4skfmz1OW9jxxwyaR/41VYzPVy\n"
        "w7m0rsL9n3v5PCFJ1zrldYayKq9gOdwOZInNJQgPU6imXEFBZWdqdR+xdL7qHVMk\n"
        "dzBRoFqnYjr2bonw87Rxk7dAsdzmUUPJTIayzDTdtU78XkOl0FivJWBXDQPiVB2Z\n"
        "5NdOVPUANDnADA5octuC3YTG2ncpG9aNKM4ggAyr3imdnz5UZdioYkecv2TVJHT2\n"
        "lXE5PH/ifJs8p9+WFgyzFsW97BUF1nKsZiUWvlyPY9C9PKuhI4OMeTEA99YIRXYj\n"
        "XDSZq300ko5sXUVKVG2osAR4Sr3wqS5hLGNBfym7FZwsQ+/ja+fenEALAbAiEMpI\n"
        "9h71lpMpIhdx7rdRznNeSOok/pVMaqvBZ0IvAq+yXb7d8C+tAND0IcUxYyyJqkE7\n"
        "89RJdcGZO9JMXv4inBFIEuMAMlGPxzcroEx+ehDXyGR0asw7CooV2DCfiEJlIsKz\n"
        "LcPZew==\n"
        "-----END CERTIFICATE-----\n";

    static ArduinoJson::JsonDocument weatherJson;

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
