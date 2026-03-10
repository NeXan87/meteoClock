#pragma once
#include "config.h"

namespace LED {

// Инициализация RGB-светодиода
void init();

// Установить цвет (0..255) для каждого канала
void setColor(uint8_t r, uint8_t g, uint8_t b);

// Обновить состояние (мигание, яркость)
void update();

// Установить режим привязки (0-co2,1-hum,2-temp,3-rain)
void setMode(uint8_t m);

// Установить яркость (0-11, 11 - авто)
void setBrightness(uint8_t b);

}  // namespace LED
