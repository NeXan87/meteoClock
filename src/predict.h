#pragma once

#include <Arduino.h>

bool initPredict();
void updatePredict();

int getPredictRain();  // возвращает dispRain (в %)
