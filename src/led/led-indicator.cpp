#include "led/led-indicator.h"

#include <Arduino.h>

#include "config.h"
#include "sensors/sensors.h"

namespace LED {

static uint8_t brightness = LED_BRIGHT_MIN;  // 0..11 special
static uint8_t mode = 0;                     // привязка
static uint8_t ledOn = 0;
static bool isBlinking = false;
static unsigned long lastBlink = 0;

void init() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_COM, OUTPUT);
    analogWrite(LED_COM, 0);
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

void setMode(uint8_t m) {
    mode = m;
}

void setBrightness(uint8_t b) {
    brightness = b;
}

static void computeBrightness() {
    if (brightness == 11) {  // авто
        int light = analogRead(PHOTO);
        if (light < BRIGHT_THRESHOLD) {
            ledOn = LED_BRIGHT_MIN;
        } else {
            ledOn = LED_BRIGHT_MAX;
        }
    } else {
        ledOn = brightness * brightness * 2.5;
    }
}

void update() {
    computeBrightness();

    // выбор цвета в зависимости от режима и текущих показаний
    int val = 0;
    switch (mode) {
        case 0: val = Sensors::getCO2(); break;
        case 1: val = Sensors::getHumidity(); break;
        case 2: val = Sensors::getTemp(); break;
        case 3: val = Sensors::getRain(); break;
    }
    // простая логика: зеленый нормальный, желтый средний, красный высокий
    uint8_t r = 0, g = 0, b = 0;
    if (mode == 0) {
        if (val < NORM_CO2) {
            g = ledOn;
        } else if (val < MAX_CO2) {
            r = ledOn;
            g = ledOn / 2;
        } else {
            r = ledOn;
        }
        if (val >= BLINK_LED_CO2 && (millis() - lastBlink > 500)) {
            isBlinking = !isBlinking;
            lastBlink = millis();
        }
    }
    // остальные режимы можно реализовать аналогично
    if (isBlinking) {
        applyColor(0, 0, 0);
    } else {
        applyColor(r, g, b);
    }
}

}  // namespace LED
