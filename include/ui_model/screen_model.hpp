#pragma once
#include <ctime>
#include <vector>
#include "util/debouncer.hpp"
#include "services/weather_service.hpp"
#include "services/hsl_service.hpp"

namespace UiModel
{
    class ChangeTracker
    {
    public:
        bool hasChanged() const;
        void setChanged(bool changed);

    private:
        bool changed = false;
    };

    class Clock : public ChangeTracker
    {
    public:
        Clock();
        void update();
        time_t getTime() const;

    private:
        Util::Debouncer debouncer;
        time_t current_time = 0;
    };

    class Weather : public ChangeTracker
    {
    public:
        Weather();
        void update();
        const std::vector<Services::WeatherService::WeatherForecast> &getForecasts() const;

    private:
        Util::Debouncer debouncer;
        std::vector<Services::WeatherService::WeatherForecast> forecasts;
    };

    class Schedule : public ChangeTracker
    {
    public:
        Schedule();
        void update();
        const std::vector<Services::HslService::StopTime> &getStopTimes() const;

    private:
        Util::Debouncer fetchDebouncer;
        Util::Debouncer screenUpdateDebouncer;
        std::vector<Services::HslService::StopTime> stopTimes;

        void fetchNewStopTimes();
    };

    class Screen
    {
    public:
        void updateAll();
        void updateClock();

        bool hasChanged() const;
        void resetChanged();

        Clock &getClock();
        const Clock &getClock() const;
        Weather &getWeather();
        const Weather &getWeather() const;
        Schedule &getSchedule();
        const Schedule &getSchedule() const;

    private:
        Clock clock;
        Weather weather;
        Schedule schedule;
    };
};