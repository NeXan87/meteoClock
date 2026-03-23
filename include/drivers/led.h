#pragma once

#include <Arduino.h>

#include "config.h"

namespace LED {

/**
 * Инициализация пинов RGB-светодиода
 */
void init();

/**
 * Установить уровень общего вывода (COM)
 * @param level Уровень 0-255 (PWM)
 */
void setCommon(uint8_t level);

/**
 * Установить сырые значения для каждого канала (0-255)
 * Учитывает тип светодиода (общий катод/анод)
 * @param r Красный канал
 * @param g Зелёный канал
 * @param b Синий канал
 */
void setRawColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * Получить состояние общего вывода
 * @return true если активен (для отладки)
 */
bool isCommonActive();

}  // namespace LED
