#pragma once

#include <Arduino.h>
#include "RTClib.h"

// Внешние переменные
extern int8_t hrs, mins, secs;
extern RTC_DS3231 rtc;
extern DateTime now;
extern boolean dotFlag;

// Функции
void clockInit();
void clockTick();
void clockLoad();