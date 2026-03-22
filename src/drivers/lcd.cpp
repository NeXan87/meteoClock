#include "drivers/lcd.h"

#include <LiquidCrystal_I2C.h>
#include <stdarg.h>

#include "display/display.h"

namespace LCD {

static LiquidCrystal_I2C lcd(DISPLAY_ADDR, LCD_COLS, LCD_ROWS);

static char lcdPrintfBuffer[32];

void init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    pinMode(BACKLIGHT, OUTPUT);
    analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
    Display::createCustomChars();
    if (DEBUG) {
        Serial.println("Display::init called");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Display init");
    }
}

void clear() {
    lcd.clear();
}

void setCursor(uint8_t col, uint8_t row) {
    if (col < LCD_COLS && row < LCD_ROWS) {
        lcd.setCursor(col, row);
    }
}

void print(const char* str) {
    lcd.print(str);
}

void printInt(long value) {
    lcd.print(value);
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int written = vsnprintf(lcdPrintfBuffer, sizeof(lcdPrintfBuffer), format, args);
    va_end(args);

    if (written > 0) {
        lcd.print(lcdPrintfBuffer);
    }
}

void write(uint8_t c) {
    lcd.write(c);
}

void createChar(uint8_t location, uint8_t* charmap) {
    lcd.createChar(location, charmap);
}

}  // namespace LCD
