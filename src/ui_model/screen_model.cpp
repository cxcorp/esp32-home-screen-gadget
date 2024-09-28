#include "ui_model/screen_model.hpp"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <sstream>
#include <iomanip>
#include "time_utils.hpp"

namespace UiModel
{
    static void reconnectWiFiIfDisconnected()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            return;
        }

        WiFi.reconnect();
        for (size_t waits = 0; waits < 100; waits++)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                break;
            }

            delay(200);
        }
    }

    bool ChangeTracker::hasChanged() const
    {
        return this->changed;
    }

    void ChangeTracker::setChanged(bool changed)
    {
        this->changed = changed;
    }

    Clock::Clock() : debouncer(900) {}

    void Clock::update()
    {
        if (!this->debouncer.tryElapse())
        {
            return;
        }

        time(&this->current_time);
        this->setChanged(true);
    }

    time_t Clock::getTime() const
    {
        return this->current_time;
    }

    Weather::Weather() : debouncer(30 * TimeUtils::MIN_TO_S * TimeUtils::S_TO_MS) {}

    void Weather::update()
    {
        if (!this->debouncer.tryElapse())
        {
            return;
        }

        std::vector<Services::WeatherService::WeatherForecast> forecasts;
        if (!Services::WeatherService::getWeatherForecasts(forecasts))
        {
            reconnectWiFiIfDisconnected();
        }

        forecasts.clear();
        if (!Services::WeatherService::getWeatherForecasts(forecasts))
        {
            return;
        }

        this->forecasts = std::move(forecasts);
        this->setChanged(true);
    }

    const std::vector<Services::WeatherService::WeatherForecast> &Weather::getForecasts() const
    {
        return this->forecasts;
    }

    Schedule::Schedule()
        : fetchDebouncer(30 * TimeUtils::S_TO_MS),
          screenUpdateDebouncer(2 * TimeUtils::S_TO_MS)
    {
    }

    void Schedule::update()
    {
        if (this->fetchDebouncer.tryElapse())
        {
            this->fetchNewStopTimes();
        }

        if (this->screenUpdateDebouncer.tryElapse())
        {
            this->setChanged(true);
        }
    }

    void Schedule::fetchNewStopTimes()
    {
        Serial.println("[UiModel::Schedule::fetchNewStopTimes] Fetching schedule");

        std::vector<Services::HslService::StopTime> stopTimes;

        if (!Services::HslService::getSchedule(stopTimes))
        {
            Serial.println("[UiModel::Schedule::update] HslService::getSchedule failed, try reconnect WiFi");
            reconnectWiFiIfDisconnected();

            stopTimes.clear();

            Serial.println("[UiModel::Schedule::update] Try get schedule after WiFi reconnect");

            if (!Services::HslService::getSchedule(stopTimes))
            {
                Serial.println("[UiModel::Schedule::update] Schedule failed to update after WiFi reconnect, trying again later");
                return;
            }
        }

        this->stopTimes = std::move(stopTimes);
        this->setChanged(true);
    }

    const std::vector<Services::HslService::StopTime> &Schedule::getStopTimes() const
    {
        return this->stopTimes;
    }

    void Screen::updateAll()
    {
        this->clock.update();
        this->weather.update();
        this->schedule.update();
    }

    void Screen::updateClock()
    {
        this->clock.update();
    }

    bool Screen::hasChanged() const
    {
        return this->clock.hasChanged() || this->weather.hasChanged() || this->schedule.hasChanged();
    }

    void Screen::resetChanged()
    {
        this->clock.setChanged(false);
        this->weather.setChanged(false);
        this->schedule.setChanged(false);
    }

    Clock &Screen::getClock()
    {
        return this->clock;
    }

    const Clock &Screen::getClock() const
    {
        return this->clock;
    }

    Weather &Screen::getWeather()
    {
        return this->weather;
    }

    const Weather &Screen::getWeather() const
    {
        return this->weather;
    }

    Schedule &Screen::getSchedule()
    {
        return this->schedule;
    }

    const Schedule &Screen::getSchedule() const
    {
        return this->schedule;
    }
};
