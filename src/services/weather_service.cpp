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

namespace Services
{
  namespace WeatherService
  {
    static const char *opendata_fmi_fi_tls_root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
        "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
        "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
        "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
        "MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
        "BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
        "aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
        "dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
        "AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
        "3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
        "tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
        "Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
        "VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
        "79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
        "c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
        "Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
        "c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
        "UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
        "Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
        "BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
        "A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
        "Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
        "VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
        "ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
        "8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
        "iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
        "Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
        "XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
        "qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
        "VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
        "L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
        "jjxDah2nGN59PRbxYvnKkKj9\n"
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
         << "?latlon=60.1554,24.9105"
         << "&param=epochtime,temperature,smartsymbol"
         << "&timestep=" << (3 * 60)
         << "&tz=Europe%2FHelsinki"
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
