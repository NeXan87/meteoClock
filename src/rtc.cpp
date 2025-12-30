#include "rtc.h"

#include <RTClib.h>

#include "config.h"

static RTC_DS3231 rtc;
static DateTime now;

bool initRtc() {
    if (!rtc.begin()) {
        return false;  // аппаратная ошибка: не удалось инициализировать I²C
    }

    if (RESET_CLOCK == 1 || rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    updateRtc();  // первое чтение
    return true;
}

void updateRtc() {
    now = rtc.now();
}

uint8_t getRtcHours() {
    return now.hour();
}

uint8_t getRtcMinutes() {
    return now.minute();
}

uint8_t getRtcSeconds() {
    return now.second();
}

uint8_t getRtcDay() {
    return now.day();
}

uint8_t getRtcMonth() {
    return now.month();
}

uint16_t getRtcYear() {
    return now.year();
}

uint8_t getRtcDayOfWeek() {
    return now.dayOfTheWeek();
}
