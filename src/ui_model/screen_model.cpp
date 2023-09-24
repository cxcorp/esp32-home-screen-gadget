#include "ui_model/screen_model.hpp"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <sstream>
#include <iomanip>
#include "time_utils.hpp"

namespace UiModel
{
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
            std::vector<Services::HslService::StopTime> stopTimes;
            if (Services::HslService::getSchedule(stopTimes))
            {
                this->stopTimes = std::move(stopTimes);
                this->setChanged(true);
            }
        }

        if (this->screenUpdateDebouncer.tryElapse())
        {
            this->setChanged(true);
        }
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

    const Clock &Screen::getClock() const
    {
        return this->clock;
    }

    const Weather &Screen::getWeather() const
    {
        return this->weather;
    }

    const Schedule &Screen::getSchedule() const
    {
        return this->schedule;
    }
};
