#pragma once

#include <Arduino.h>

#include "config.h"
#include "ui/enums.h"

namespace LEDController {
/**
 * Установить цвет на основе текущего режима и показаний датчиков
 * Вызывается в основном цикле loop()
 */
void update();

/**
 * Установить режим привязки к датчику
 * @param m Режим привязки (CO2, Humidity, Temperature, Rain)
 */
void setMode(UI::LEDBindMode m);

/**
 * Установить яркость
 * @param b 0-10 - ручная яркость, 11 - автоматическая
 */
void setBrightness(uint8_t b);

/**
 * Получить текущую яркость
 * @return 0-10 - ручная, 11 - авто
 */
uint8_t getBrightness();

/**
 * Получить текущий режим привязки
 * @return Режим привязки
 */
UI::LEDBindMode getMode();

/**
 * Получить текущий статус тревоги
 * @return Статус (Normal, Warning, Critical, Blinking)
 */
UI::AlertStatus getAlertStatus();

/**
 * Принудительно установить цвет (для ручного режима)
 * @param r Красный канал (0-255)
 * @param g Зелёный канал (0-255)
 * @param b Синий канал (0-255)
 */
void setColor(uint8_t r, uint8_t g, uint8_t b);

}  // namespace LEDController
