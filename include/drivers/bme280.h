#pragma once
#include <Arduino.h>

#include "config.h"

namespace BME280 {

// Инициализация датчиков
void init();

// Считать значения с датчиков, обновляет внутреннее состояние
void update();

// Получить последнее значение
float getTemp();
float getHumidity();
float getPres();
float getCO2();
float getAlt();
int getRain();
void setRain(int r);

}  // namespace BME280
