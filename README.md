# esp32-clock-weather-bus-schedule-screen

Firmware for a ESP32-2432S032 2.8" TFT (resistive) touchscreen module to provide the following features:
- Clock
  * Synchronized via NTP
- Weather for the next 15 hours
  * Temperature and weather symbol, courtesy of the Finnish Meteorological Institute
- Public transport schedule for a couple of near bus/tram stops
  * Similar in style to the schedules you see on bus stops nowadays (https://omatnaytot.hsl.fi/createview).

The UI is implemented with LVGL. Their SquareLine Studio software was used to design the UIs, and export the bare UI elements into code. Based on the [esp32-smartdisplay library](https://github.com/rzeldent/esp32-smartdisplay)'s LVGL configs.

## Images

|  **UI design** |
| --- |
|  ![](docs/ui.png)  |
| |
| **Mounted on the wall** |
| ![](docs/on_the_wall.jpg) |



## Licenses

### Code

MIT License. See LICENSE.

### Data

- FMI's open weather data is licensed under CC-BY-4.0. Ilmatieteen laitoksen avoin data. https://en.ilmatieteenlaitos.fi/open-data-licence.
- Public transport data comes from Digitransit's APIs. © Digitransit 2023 licensed under [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/).

