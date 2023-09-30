#include <Arduino.h>
#include <esp_sntp.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <esp32_smartdisplay.h>
#include <functional>

#include "secrets.h"
#include "ui/ui.h"
#include "ui_model/screen_model.hpp"
#include "device/ntp.hpp"
#include "presenter.hpp"
#include "time_utils.hpp"

static constexpr uint16_t BACKLIGHT_ACTIVE = ILI9341_PWM_MAX_BL;
static constexpr uint16_t BACKLIGHT_DIMMED = 64;
static constexpr uint16_t BACKLIGHT_SLEEPING = 00;

static UiModel::Screen screenModel;

static StaticTask_t xLvglTaskBuffer;
// https://docs.lvgl.io/latest/en/html/intro/index.html#requirements
// ">8kB is recommended"
static constexpr size_t xLvglTaskStackSize = 10240;
static StackType_t xLvglTaskStack[xLvglTaskStackSize];
static TaskHandle_t xLvglTaskHandle;

static void updateStateAndPresent();
static void lvglTimerTaskCode(void *parameter);

static void updateInitScreenLabel(const char *text)
{
    const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
    lv_label_set_text(ui_init_screen_label, text);
}

void setup()
{
    Serial.begin(115200);

    smartdisplay_init();
    // turn off LED on the back side
    smartdisplay_set_led_color(lv_color32_t{.full = 0});

    // clear the screen
    lv_obj_clean(lv_scr_act());
    ui_init();

    // spawn a FreeRTOS task to update the screen so that our
    // blocking HTTP requests aren't a problem
    xLvglTaskHandle = xTaskCreateStatic(
        lvglTimerTaskCode,
        "LVGLTimerTask",
        xLvglTaskStackSize,
        (void *)0xFDFDFDFD,
        1,
        xLvglTaskStack,
        &xLvglTaskBuffer);

    updateInitScreenLabel("Connecting to WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

    Serial.println("Waiting for WiFi to connect...");
    while ((WiFi.status() != WL_CONNECTED))
    {
        yield();
    }
    Serial.println("\nConnected!");

    ArduinoOTA.setPassword(ARDUINO_OTA_PASSWORD);
    ArduinoOTA.onStart(
                  []()
                  {
                      Serial.printf("[OTA] onStart: %d\n", ArduinoOTA.getCommand());
                  })
        .onEnd(
            []()
            {
                Serial.println("[OTA] onEnd");
            })
        .onProgress(
            [](unsigned int progress, unsigned int total)
            {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            })
        .onError(
            [](ota_error_t error)
            {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR)
                    Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR)
                    Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR)
                    Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR)
                    Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR)
                    Serial.println("End Failed");
            });
    ArduinoOTA.begin();

    updateInitScreenLabel("Synchronizing clock...");
    NTP::setup();
    NTP::block_until_synced(yield);

    updateInitScreenLabel("Getting things ready...");
    // update data on the main screen, then switch to it
    updateStateAndPresent();

    {
        const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
        lv_disp_load_scr(ui_main_screen);
    }

    smartdisplay_tft_set_backlight(BACKLIGHT_ACTIVE);
}

static uint32_t lvglTimerExecuteTaskHandler()
{
    const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
    return lv_task_handler();
}

static void lvglTimerTaskCode(void *parameter)
{
    for (;;)
    {
        uint32_t timeUntilNextCall = lvglTimerExecuteTaskHandler();
        if (timeUntilNextCall == LV_NO_TASK_READY)
        {
            timeUntilNextCall = 1;
        }

        const TickType_t xDelay = timeUntilNextCall / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }
}

static uint32_t getDisplayLastActivityTime()
{
    const std::lock_guard<std::recursive_mutex> lock(lvgl_mutex);
    return lv_disp_get_default()->last_activity_time;
}

static void updateStateAndPresent()
{
    screenModel.updateAll();
    Presenter::present_screen(screenModel);
    screenModel.resetChanged();
}

enum ActivityState
{
    ACTIVITY_STATE_ACTIVE,
    ACTIVITY_STATE_SCREEN_DIMMED,
    ACTIVITY_STATE_SLEEPING
};

ActivityState state = ACTIVITY_STATE_ACTIVE;

static uint32_t lastActivityTimeWhenScreenWasDimmed = 0;
/**
 * How many ms should we wait since the last time the screen was interacted
 * with until we dim the screen?
 */
static constexpr uint32_t DIM_SCREEN_AFTER_MS = 5 * TimeUtils::MIN_TO_S * TimeUtils::S_TO_MS;
/**
 * How many ms should we wait since the last time the screen was interacted
 * with until we go to sleep? If less than or equal to `dimScreenAfterMs`,
 * we skip the screen dimming and just go to sleep. */
static constexpr uint32_t SLEEP_AFTER_MS = DIM_SCREEN_AFTER_MS + 30 * TimeUtils::S_TO_MS;

void animateBacklight(uint16_t from, uint16_t to, size_t steps, uint32_t frameTime)
{
    for (size_t i = 0; i < steps; ++i)
    {
        smartdisplay_tft_set_backlight(
            map(i, 0, steps - 1, from, to));
        delay(frameTime);
    }
}

static void transitionActiveToDimmed(uint32_t lastActivityTime)
{
    animateBacklight(BACKLIGHT_ACTIVE, BACKLIGHT_DIMMED, 32, 33);

    lastActivityTimeWhenScreenWasDimmed = lastActivityTime;
    state = ACTIVITY_STATE_SCREEN_DIMMED;
}

static void transitionDimmedToActive()
{
    smartdisplay_tft_set_backlight(ILI9341_PWM_MAX_BL);
    state = ACTIVITY_STATE_ACTIVE;
}

static void transitionDimmedToSleeping()
{
    animateBacklight(BACKLIGHT_DIMMED, BACKLIGHT_SLEEPING, 32, 33);
    smartdisplay_tft_sleep();
    // idk if the display needs some time to process the sleep command,
    // let's just sleep for a bit in case
    delay(200);

    state = ACTIVITY_STATE_SLEEPING;
}

static void transitionSleepingToActive()
{
    smartdisplay_tft_wake();
    // Just update the clock since it's instant then show whatever
    // data we have even if it's stale.
    screenModel.updateClock();
    Presenter::present_screen(screenModel);
    screenModel.resetChanged();

    animateBacklight(BACKLIGHT_SLEEPING, BACKLIGHT_ACTIVE, 16, 33);

    state = ACTIVITY_STATE_ACTIVE;
}

static void loopActive()
{
    updateStateAndPresent();

    const uint32_t lastActivityTime = getDisplayLastActivityTime();
    const uint32_t timeSinceLastActivity = lv_tick_elaps(lastActivityTime);

    if (timeSinceLastActivity > DIM_SCREEN_AFTER_MS)
    {
        Serial.printf(
            "[loopActive] timeSinceLastActivity > dimScreenAfterMs, transitioning to Dimmed. lastActivityTime: %u, lastActivityTimeWhenScreenWasDimmed: %u, timeSinceLastActivity: %u, dimScreenAfterMs: %u\n",
            lastActivityTime,
            lastActivityTimeWhenScreenWasDimmed,
            timeSinceLastActivity,
            DIM_SCREEN_AFTER_MS);
        transitionActiveToDimmed(lastActivityTime);
    }
}

static void loopDimmed()
{
    updateStateAndPresent();

    const uint32_t lastActivityTime = getDisplayLastActivityTime();
    if (lastActivityTime != lastActivityTimeWhenScreenWasDimmed)
    {
        Serial.printf(
            "[loopDimmed] Activity! Transitioning to Active. lastActivityTime: %u, lastActivityTimeWhenScreenWasDimmed: %u\n",
            lastActivityTime,
            lastActivityTimeWhenScreenWasDimmed);
        // there's been activity since the screen was dimmed!
        transitionDimmedToActive();
        return;
    }

    const uint32_t timeSinceLastActivity = lv_tick_elaps(lastActivityTime);
    if (timeSinceLastActivity > SLEEP_AFTER_MS)
    {
        Serial.printf(
            "[loopDimmed] timeSinceLastActivity > sleepAfterMs, transitioning to Sleeping. lastActivityTime: %u, lastActivityTimeWhenScreenWasDimmed: %u, timeSinceLastActivity: %u, sleepAfterMs: %u\n",
            lastActivityTime,
            lastActivityTimeWhenScreenWasDimmed,
            timeSinceLastActivity,
            SLEEP_AFTER_MS);
        transitionDimmedToSleeping();
    }
}

static void loopSleeping()
{
    static Util::Debouncer backgroundUpdateDebouncer(1 * TimeUtils::HOUR_TO_S * TimeUtils::S_TO_MS);
    if (backgroundUpdateDebouncer.tryElapse())
    {
        // let the model update once an hour in the background so that we
        // have at least *somewhat* fresh data to show to the user in the morning
        // (for the weather, mostly)
        screenModel.updateAll();
    }

    const uint32_t lastActivityTime = getDisplayLastActivityTime();
    if (lastActivityTime != lastActivityTimeWhenScreenWasDimmed)
    {
        // there's been activity since we started sleeping!
        transitionSleepingToActive();
        Serial.printf(
            "[loopSleeping] Activity! Transitioning to Active. lastActivityTime: %u, lastActivityTimeWhenScreenWasDimmed: %u\n",
            lastActivityTime,
            lastActivityTimeWhenScreenWasDimmed);
        return;
    }
}

void loop()
{
    ArduinoOTA.handle();

    switch (state)
    {
    case ACTIVITY_STATE_ACTIVE:
        loopActive();
        return;
    case ACTIVITY_STATE_SCREEN_DIMMED:
        loopDimmed();
        return;
    case ACTIVITY_STATE_SLEEPING:
        loopSleeping();
        return;
    default:
        return;
    }
}