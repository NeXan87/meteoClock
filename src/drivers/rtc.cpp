#include "drivers/rtc.h"

#include <Arduino.h>
#include <RTClib.h>

#include "config.h"
#include "display/display.h"  // для обновления экрана при необходимости
#include "drivers/bme280.h"
#include "ui/enums.h"
#include "ui/ui.h"

namespace RTC {

static RTC_DS3231 rtc;
static DateTime now;
static int hrs = 0, mins = 0, secs = 0;
static bool isDotOnFlag = false;

void init() {
    rtc.begin();
    if (RESET_CLOCK) {
        now = rtc.now();
        // настройки по умолчанию
    }
    now = rtc.now();
    hrs = now.hour();
    mins = now.minute();
    secs = now.second();
}

void tick() {
    isDotOnFlag = !isDotOnFlag;
    if (isDotOnFlag) {
        secs++;
        if (secs > 59) {
            secs = 0;
            mins++;
        }
        if (mins > 59) {
            now = rtc.now();
            secs = now.second();
            mins = now.minute();
            hrs = now.hour();
            if (hrs > 23) hrs = 0;
        }
    }
    // всегда обновляем экран, если мы на главном режиме
    if (UI::currentMode() == static_cast<uint8_t>(UI::Mode::Clock)) {
        Display::drawSensors();
    }
}

int getHours() { return hrs; }
int getMinutes() { return mins; }
int getSeconds() { return secs; }

bool isDotOn() { return isDotOnFlag; }

}  // namespace RTC
