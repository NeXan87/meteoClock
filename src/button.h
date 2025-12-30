#pragma once

#include <Arduino.h>

#include "GyverButton.h"

// Обновление состояния кнопки
void updateButton();

// Флаги событий
bool isSingleButton();
bool isDoubleButton();
bool isTripleButton();
bool isHoldedButton();
