# esp32-clock-weather-bus-schedule-screen

Firmware for a ESP32-2432S032 2.8" TFT (resistive) touchscreen module to provide the following features:
- Clock
  * Synchronized via NTP
- Weather for the next 15 hours
  * Temperature and weather symbol, courtesy of the Finnish Meteorological Institute
- Public transport schedule for a couple of near bus/tram stops
  * Similar in style to the schedules you see on bus stops nowadays (https://omatnaytot.hsl.fi/createview).


Based on the esp32-smartdisplay library's LVGL configs.

## Licenses

### Code

MIT License. See LICENSE.

### Data

- FMI's open weather data is licensed under CC-BY-4.0. Ilmatieteen laitoksen avoin data. https://en.ilmatieteenlaitos.fi/open-data-licence.
- Public transport data comes from Digitransit's APIs. © Digitransit 2023 licensed under [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/).

