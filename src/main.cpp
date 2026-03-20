#include <Arduino.h>
#include <EEPROM.h>

#include "button/button.h"
#include "clock/clock.h"
#include "config.h"
#include "display/display.h"
#include "led/led-indicator.h"
#include "plot/plot.h"
#include "sensors/sensors.h"
#include "ui/ui.h"

// таймеры
static unsigned long sensorsTimer = 0;
static unsigned long plotTimer = 0;
static unsigned long clockTimer = 0;

void setup() {
    // инициализация серийного порта для отладки
    Serial.begin(SERIAL_BAUD);

    Sensors::init();
    LED::init();
    Display::init();
    Button::init();
    Clock::init();
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
        Clock::tick();
    }
    Button::tick();
    UI::tick();
    LED::update();

    if (now - sensorsTimer >= SENS_TIME) {
        sensorsTimer = now;
        Sensors::update();
    }

    // обновление графиков выполняется в UI или PlotManager
    // здесь можно просто вызвать tick, если требуется
    Plot::tick();
}
