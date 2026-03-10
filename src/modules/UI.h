#pragma once
#include <Arduino.h>

#include "config.h"

namespace UI {

void init();
void tick();  // надо вызывать каждую итерацию цикла, обрабатывает нажатия

// доступ к текущему режиму
uint8_t currentMode();

// получить режим отображения на главном экране
uint8_t getMode0Scr();
bool isBigDigits();

// получить маски масштабирования/видимости графиков
int getMaxOnData();
int getVisOnData();

}  // namespace UI
