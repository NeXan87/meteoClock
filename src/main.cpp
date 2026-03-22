#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"
#include "display/display.h"
#include "drivers/bme280.h"
#include "drivers/button.h"
#include "drivers/lcd.h"
#include "drivers/mhz19.h"
#include "drivers/rtc.h"
#include "led/led-indicator.h"
#include "plot/plot.h"
#include "ui/ui.h"

// таймеры
static unsigned long sensorsTimer = 0;
static unsigned long plotTimer = 0;
static unsigned long clockTimer = 0;

void setup() {
    // инициализация серийного порта для отладки
    Serial.begin(SERIAL_BAUD);

#if (CO2_SENSOR == 1)
    MHZ19::init();
#endif
    BME280::init();
    LED::init();
    LCD::init();
    RTC::init();
    Plot::init();
    UI::init();

    sensorsTimer = millis();
    plotTimer = millis();
    clockTimer = millis();
}

void loop() {
    unsigned long now = millis();

    Display::tick();
    // тик часов — не чаще, чем раз в CLOCK_TICK_INTERVAL_MS
    if (now - clockTimer >= CLOCK_TICK_INTERVAL_MS) {
        clockTimer = now;
        RTC::tick();
    }
    Button::tick();
    UI::tick();
    LED::update();

    if (now - sensorsTimer >= SENS_TIME) {
        sensorsTimer = now;
        BME280::update();
    }

    // обновление графиков выполняется в UI или PlotManager
    // здесь можно просто вызвать tick, если требуется
    Plot::tick();
}
