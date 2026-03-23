#include "led/led-controller.h"

#include <Arduino.h>

#include "config.h"
#include "drivers/bme280.h"
#include "drivers/led.h"
#include "drivers/mhz19.h"
#include "ui/enums.h"

namespace LEDController {

// Состояние контроллера
static uint8_t brightness = LED_BRIGHT_MIN;  // 0..10 - ручная яркость
static bool isAutoBrightness = true;         // true = авто, false = ручная
static UI::LEDBindMode bindMode = UI::LEDBindMode::CO2;
static uint8_t ledOn = 0;
static bool isBlinking = false;
static unsigned long lastBlink = 0;
static UI::AlertStatus alertStatus = UI::AlertStatus::Normal;

// для автояркости с гистерезисом
static bool isDark = false;
static unsigned long brightTimer = 0;

// для ручного установки цвета
static bool isManualColor = false;
static uint8_t manualR = 0;
static uint8_t manualG = 0;
static uint8_t manualB = 0;

/**
 * Проверка яркости по фоторезистору
 * Вызывается каждые 2 секунды
 */
static void checkBrightness() {
    int photoValue = analogRead(PHOTO);

    // гистерезис как в оригинале
    if (isDark) {
        // Сейчас темно → переключаемся в "светло", только если сигнал УВЕРЕННО выше порога
        if (photoValue > BRIGHT_THRESHOLD + BRIGHT_HYSTERESYS) {
            isDark = false;
        }
    } else {
        // Сейчас светло → переключаемся в "темно", только если сигнал УВЕРЕННО ниже порога
        if (photoValue < BRIGHT_THRESHOLD - BRIGHT_HYSTERESYS) {
            isDark = true;
        }
    }

    if (isAutoBrightness) {
        if (isDark) {
#if (LED_MODE == 0)
            ledOn = LED_BRIGHT_MIN;
#else
            ledOn = 255 - LED_BRIGHT_MIN;
#endif
        } else {
#if (LED_MODE == 0)
            ledOn = LED_BRIGHT_MAX;
#else
            ledOn = 255 - LED_BRIGHT_MAX;
#endif
        }
    }
    // если не авто - ledOn уже установлен в setBrightness()

    if (DEBUG) {
        Serial.print("photo=");
        Serial.print(photoValue);
        Serial.print(" isDark=");
        Serial.print(isDark);
        Serial.print(" ledOn=");
        Serial.println(ledOn);
    }
}

/**
 * Расчёт цвета на основе текущего режима и показаний датчиков
 */
static void calculateColor(uint8_t& r, uint8_t& g, uint8_t& b) {
    r = 0;
    g = 0;
    b = 0;

    // если ручной режим цвета - используем его
    if (isManualColor) {
        r = manualR;
        g = manualG;
        b = manualB;
        return;
    }

    // выбор режима и получение значения
    int val = 0;
    switch (bindMode) {
        case UI::LEDBindMode::CO2:
#if (CO2_SENSOR == 1)
            val = MHZ19::getCO2();
#else
            val = 0;
#endif
            break;
        case UI::LEDBindMode::Humidity:
            val = static_cast<int>(BME280::getHumidity());
            break;
        case UI::LEDBindMode::Temperature:
            val = static_cast<int>(BME280::getTemp());
            break;
        case UI::LEDBindMode::Rain:
            val = BME280::getRain();
            break;
    }

    // логика для CO2
    if (bindMode == UI::LEDBindMode::CO2) {
        if (val < NORM_CO2) {
            g = ledOn;
            alertStatus = UI::AlertStatus::Normal;
        } else if (val < MAX_CO2) {
            b = ledOn;
            alertStatus = UI::AlertStatus::Warning;
        } else {
            r = ledOn;
            alertStatus = UI::AlertStatus::Critical;
        }

        // проверка на мигание
        if (val >= BLINK_LED_CO2) {
            if (millis() - lastBlink > LED_BLINK_INTERVAL_MS) {
                isBlinking = !isBlinking;
                lastBlink = millis();
            }
            if (isBlinking) {
                alertStatus = UI::AlertStatus::Blinking;
            }
        }
    }
    // TODO: добавить логику для других режимов (Temperature, Humidity, Rain)
}

void update() {
    // вызываем checkBrightness каждые LED_BRIGHT_CHECK_INTERVAL_MS
    unsigned long now = millis();
    if (now - brightTimer >= LED_BRIGHT_CHECK_INTERVAL_MS) {
        brightTimer = now;
        checkBrightness();
    }

    if (DEBUG) {
        Serial.print("LED update: brightness=");
        Serial.print(brightness);
        Serial.print(" ledOn=");
        Serial.println(ledOn);
    }

    // расчёт цвета
    uint8_t r, g, b;
    calculateColor(r, g, b);

    // применение цвета через драйвер
    if (isBlinking || alertStatus == UI::AlertStatus::Blinking) {
        LED::setRawColor(0, 0, 0);
    } else {
        LED::setRawColor(r, g, b);
    }
}

void setMode(UI::LEDBindMode m) {
    bindMode = m;
    isManualColor = false;
}

void setBrightness(uint8_t b) {
    if (b == 11) {
        isAutoBrightness = true;
    } else {
        isAutoBrightness = false;
        brightness = b;
        // расчёт ledOn для ручного режима
        ledOn = static_cast<uint8_t>(b * b * 2.5);
    }
}

uint8_t getBrightness() {
    return isAutoBrightness ? 11 : brightness;
}

UI::LEDBindMode getMode() {
    return bindMode;
}

UI::AlertStatus getAlertStatus() {
    return alertStatus;
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
    isManualColor = true;
    manualR = r;
    manualG = g;
    manualB = b;
}

}  // namespace LEDController
