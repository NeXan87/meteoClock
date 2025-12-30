#pragma once

#include <Arduino.h>

bool initBme();
void updateBme();

float getBmeTemperature();
float getBmeHumidity();
float getBmePressure();  // возвращается в Pa
float getBmeAltitude();  // в метрах
