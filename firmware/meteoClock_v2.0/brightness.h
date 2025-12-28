#pragma once

#include <Arduino.h>

// Внешние переменные (определены в brightness.cpp или eeprom.cpp)
extern byte LCD_BRIGHT;
extern byte LED_BRIGHT;
extern byte LED_ON;
extern byte LED_OFF;

// Функции
void brightnessInit();
void brightnessUpdate();