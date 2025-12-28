#pragma once

#include <Arduino.h>

// Внешние переменные
extern byte LED_ON;
extern byte LED_OFF;
extern byte LEDType;

// Функции
void ledInit();
void setLED();
void setLEDcolor(byte color);