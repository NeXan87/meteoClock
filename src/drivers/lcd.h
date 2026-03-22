#pragma once

#include <Arduino.h>

#include "config.h"

namespace LCD {
void init();
void clear();
void setCursor(uint8_t col, uint8_t row);
void print(const char* str);
void printInt(long value);
void printf(const char* format, ...);
void write(uint8_t c);
void createChar(uint8_t location, uint8_t* charmap);
}  // namespace LCD
