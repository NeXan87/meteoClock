#pragma once

#include <Arduino.h>

// Сигнатура EEPROM
#define EEPROM_SIG_VALUE 122

// Внешние переменные (определяются в eeprom.cpp)
extern int MAX_ONDATA;
extern int VIS_ONDATA;
extern byte mode0scr;
extern boolean bigDig;
extern byte LED_BRIGHT;
extern byte LCD_BRIGHT;
extern byte LEDType;

// Функции
void eepromLoadSettings();
void eepromSaveSettings();