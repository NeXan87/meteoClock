#pragma once

#include <Arduino.h>

// Внешние переменные (определены в sensors.cpp и globals.cpp)
extern int tempHour[15], tempDay[15];
extern int humHour[15], humDay[15];
extern int pressHour[15], pressDay[15];
extern int co2Hour[15], co2Day[15];
extern int altHour[15], altDay[15];
extern int dispRain;
extern int dispPres;

// Функции
void plotLoad();
void plotDraw(byte mode);
void plotDrawMenu(byte podMode);