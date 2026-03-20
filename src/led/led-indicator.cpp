#include "led/led-indicator.h"

#include <Arduino.h>

#include "config.h"
#include "sensors/sensors.h"
#include "ui/enums.h"

namespace LED {

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

void init() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_COM, OUTPUT);
    analogWrite(LED_COM, 0);
    if (DEBUG) {
        Serial.println("LED::init called");
        Serial.print("LED_MODE=");
        Serial.println(LED_MODE);
    }
}

static void applyColor(uint8_t r, uint8_t g, uint8_t b) {
    // учесть общий катод/анод
#if (LED_MODE == 0)
    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
#else
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
#endif
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
    applyColor(r, g, b);
}

void setMode(UI::LEDBindMode m) {
    bindMode = m;
}

void setBrightness(uint8_t b) {
    if (b == 11) {
        isAutoBrightness = true;
    } else {
        isAutoBrightness = false;
        brightness = b;
        ledOn = static_cast<uint8_t>(b * b * 2.5);
    }
}

uint8_t getBrightness() {
    return isAutoBrightness ? 11 : brightness;
}

UI::LEDBindMode getMode() {
    return bindMode;
}

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

void update() {
    // вызываем checkBrightness каждые 2 секунды как в оригинале
    unsigned long now = millis();
    if (now - brightTimer >= 2000) {
        brightTimer = now;
        checkBrightness();
    }

    if (DEBUG) {
        Serial.print("LED update: brightness=");
        Serial.print(brightness);
        Serial.print(" ledOn=");
        Serial.println(ledOn);
    }

    // выбор цвета в зависимости от режима и текущих показаний
    int val = 0;
    switch (bindMode) {
        case UI::LEDBindMode::CO2:
            val = Sensors::getCO2();
            break;
        case UI::LEDBindMode::Humidity:
            val = static_cast<int>(Sensors::getHumidity());
            break;
        case UI::LEDBindMode::Temperature:
            val = static_cast<int>(Sensors::getTemp());
            break;
        case UI::LEDBindMode::Rain:
            val = Sensors::getRain();
            break;
    }

    // простая логика: зелёный нормальный, жёлтый средний, красный высокий
    uint8_t r = 0, g = 0, b = 0;
    if (bindMode == UI::LEDBindMode::CO2) {
        if (val < NORM_CO2) {
            g = ledOn;
            alertStatus = UI::AlertStatus::Normal;
        } else if (val < MAX_CO2) {
            r = ledOn;
            g = ledOn / 2;
            alertStatus = UI::AlertStatus::Warning;
        } else {
            r = ledOn;
            alertStatus = UI::AlertStatus::Critical;
        }
        if (val >= BLINK_LED_CO2 && (millis() - lastBlink > LED_BLINK_INTERVAL_MS)) {
            isBlinking = !isBlinking;
            lastBlink = millis();
            alertStatus = isBlinking ? UI::AlertStatus::Blinking : UI::AlertStatus::Critical;
        }
    }
    // остальные режимы можно реализовать аналогично
    if (isBlinking || alertStatus == UI::AlertStatus::Blinking) {
        applyColor(0, 0, 0);
    } else {
        applyColor(r, g, b);
    }
}

}  // namespace LED
