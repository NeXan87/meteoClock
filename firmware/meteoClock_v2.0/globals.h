#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include "config.h"

// === Глобальные переменные времени ===
extern int8_t hrs, mins, secs;
extern RTC_DS3231 rtc;
extern DateTime now;

// === Датчики ===
extern float dispTemp;
extern byte dispHum;
extern int dispPres;
extern int dispCO2;
extern int dispRain;
extern float dispAlt;

// === Графики ===
extern int tempHour[15], tempDay[15];
extern int humHour[15], humDay[15];
extern int pressHour[15], pressDay[15];
extern int co2Hour[15], co2Day[15];
extern int altHour[15], altDay[15];
extern uint32_t pressure_array[6];

// === Настройки из EEPROM ===
extern int MAX_ONDATA;
extern int VIS_ONDATA;
extern byte mode0scr;
extern boolean bigDig;
extern byte LED_BRIGHT;
extern byte LCD_BRIGHT;
extern byte LEDType;

// === Таймеры ===
extern unsigned long sensorsTimerD;
extern unsigned long drawSensorsTimerD;
extern unsigned long clockTimerD;
extern unsigned long hourPlotTimerD;
extern unsigned long dayPlotTimerD;
extern unsigned long predictTimerD;
extern unsigned long brightTimerD;
extern unsigned long plotTimerD;

// === Режимы ===
extern byte mode;
extern byte podMode;
extern boolean dotFlag;

// === Общие функции (объявляем здесь, чтобы любой .cpp мог их вызвать) ===
void setLED();
bool testTimer(unsigned long& last, unsigned long interval);