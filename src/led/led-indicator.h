#pragma once

#include <Arduino.h>

#include "config.h"
#include "ui/enums.h"

namespace LED {

// Инициализация RGB-светодиода
void init();

// Установить цвет (0..255) для каждого канала
void setColor(uint8_t r, uint8_t g, uint8_t b);

// Обновить состояние (мигание, яркость)
void update();

// Установить режим привязки к датчику
void setMode(UI::LEDBindMode m);

// Установить яркость (0-11, 11 - авто)
void setBrightness(uint8_t b);

}  // namespace LED
