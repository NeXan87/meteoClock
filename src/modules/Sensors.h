#pragma once
#include "config.h"

namespace Sensors {

// Инициализация датчиков
void init();

// Считать значения с датчиков, обновляет внутреннее состояние
void update();

// Получить последнее значение
float getTemp();
uint8_t getHumidity();
int getPres();
int getCO2();
float getAlt();
int getRain();

// внутреннее: прогноз осадков вычисляется в PlotManager
void setRain(int r);

}  // namespace Sensors
