#pragma once

#include <Arduino.h>
#include <Wire.h>                // ← обязательно до объявления Wire
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <GyverButton.h>

#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>
#endif

// Правильный тип: TwoWire (для AVR/Uno/Nano) — это и есть Wire
extern TwoWire Wire;  // ← ИСПРАВЛЕНО: было WireClass → стало TwoWire

extern LiquidCrystal_I2C lcd;
extern GButton button;
extern RTC_DS3231 rtc;
extern Adafruit_BME280 bme;

#if (CO2_SENSOR == 1)
extern MHZ19_uart mhz19;
#endif