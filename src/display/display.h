#pragma once
#include <LiquidCrystal_I2C.h>

#include "config.h"

namespace Display {

// Инициализация дисплея
void init();

// Очистить экран
void clear();

// Создать настраиваемые символы (русские буквы, индикаторы и прочие)
void createCustomChars();

// Основной рисунок и данные (главный экран)
void drawSensors();

// Перерисовать график в соответствии с текущим режимом
void redrawPlot(uint8_t mode);

// Вспомогательные функции, используются внутри модуля
// (не экспортируются наружу)

// Обновление модуля (таймеры, автояркость и пр.)
void tick();

}  // namespace Display
