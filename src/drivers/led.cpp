#include "drivers/led.h"

#include <Arduino.h>

#include "config.h"

namespace LED {

static bool commonActive = false;

void init() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_COM, OUTPUT);
    setCommon(0);

    if (DEBUG) {
        Serial.println("LED::init called");
        Serial.print("LED_MODE=");
        Serial.println(LED_MODE);
    }
}

void setCommon(uint8_t level) {
    analogWrite(LED_COM, level);
    commonActive = (level > 0);
}

void setRawColor(uint8_t r, uint8_t g, uint8_t b) {
    // учесть общий катод/анод
#if (LED_MODE == 0)
    // общий катод: прямое управление
    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
#else
    // общий анод: инвертированное управление
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
#endif
}

bool isCommonActive() {
    return commonActive;
}

}  // namespace LED
