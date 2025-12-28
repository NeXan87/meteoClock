#include "config.h"
#include "globals.h"
#include "brightness.h"

void brightnessInit() {
    // Инициализация начальных значений яркости
    #if (LED_MODE == 0)
        LED_OFF = 0;
        LED_ON = LED_BRIGHT_MAX;
    #else
        LED_OFF = 255;
        LED_ON = (255 - LED_BRIGHT_MAX);
    #endif
}

void brightnessUpdate() {
    if (!BRIGHT_CONTROL) {
        // Яркость всегда максимальная, если управление отключено
        analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
        return;
    }

    int photoValue = analogRead(PHOTO);
    static bool isDark = false;

    if (isDark) {
        if (photoValue > BRIGHT_THRESHOLD + BRIGHT_HYSTERESYS) {
            isDark = false;
        }
    } else {
        if (photoValue < BRIGHT_THRESHOLD - BRIGHT_HYSTERESYS) {
            isDark = true;
        }
    }

    // Управление яркостью LCD
    if (LCD_BRIGHT == 11) {
        if (isDark) {
            analogWrite(BACKLIGHT, LCD_BRIGHT_MIN);
        } else {
            analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
        }
    } else {
        // Преобразуем 0–10 → 0–255 нелинейно (как в оригинале: x² * 2.5)
        byte level = (LCD_BRIGHT * LCD_BRIGHT * 25) / 10;  // = x² * 2.5, целочисленно
        analogWrite(BACKLIGHT, level);
    }

    // Подготовка значений LED_ON/LED_OFF для setLED()
    if (LED_BRIGHT == 11) {
        if (isDark) {
            #if (LED_MODE == 0)
                LED_ON = LED_BRIGHT_MIN;
            #else
                LED_ON = (255 - LED_BRIGHT_MIN);
            #endif
        } else {
            #if (LED_MODE == 0)
                LED_ON = LED_BRIGHT_MAX;
            #else
                LED_ON = (255 - LED_BRIGHT_MAX);
            #endif
        }
    } else {
        // Используется позже в setLED()
    }
}