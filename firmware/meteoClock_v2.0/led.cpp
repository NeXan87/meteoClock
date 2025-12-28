#include "config.h"
#include "globals.h"
#include "led.h"
#include "sensors.h"

#if (LED_MODE == 0)
byte LED_ON = LED_BRIGHT_MAX;
byte LED_OFF = 0;
#else
byte LED_ON = (255 - LED_BRIGHT_MAX);
byte LED_OFF = 255;
#endif

void ledInit() {
    pinMode(LED_COM, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_COM, LED_MODE);  // общий катод = LOW, общий анод = HIGH
}

void setLEDcolor(byte color) {
    // color — упакованный формат: R = color & 3, G = (color >> 2) & 3, B = (color >> 4) & 3
    byte r = (color & 3);
    byte g = (color >> 2) & 3;
    byte b = (color >> 4) & 3;

    if (LED_MODE == 0) {
        analogWrite(LED_R, r * 85);  // 0, 85, 170, 255
        analogWrite(LED_G, g * 85);
        analogWrite(LED_B, b * 85);
    } else {
        analogWrite(LED_R, 255 - r * 85);
        analogWrite(LED_G, 255 - g * 85);
        analogWrite(LED_B, 255 - b * 85);
    }
}

void setLED() {
    // Обновление LED_ON в зависимости от настройки яркости
    if (LED_BRIGHT < 11) {
        // Ручной режим: яркость = (LED_BRIGHT / 10)^2 → нелинейная шкала, как в оригинале
        byte level = (LED_BRIGHT * LED_BRIGHT * 25) / 10;  // x² * 2.5 → 0..255
        if (LED_MODE == 0) {
            LED_ON = level;
        } else {
            LED_ON = 255 - level;
        }
    } else {
        // Авто: яркость уже установлена в brightnessUpdate()
        // LED_ON актуален
    }

    // Определение цвета в зависимости от привязанного датчика (LEDType)
    byte colorCode = 0;  // по умолчанию — выключено (черный)

    if (LEDType == 0) {  // CO2
        if (dispCO2 >= maxCO2) colorCode = 3;        // красный
        else if (dispCO2 >= normCO2) colorCode = 15; // жёлтый (R+G)
        else colorCode = 12;                         // зелёный
    } else if (LEDType == 1) {  // Влажность
        if (dispHum <= minHum) colorCode = 3;        // красный
        else if (dispHum <= normHum) colorCode = 15; // жёлтый
        else if (dispHum <= maxHum) colorCode = 12;  // зелёный
        else colorCode = 48;                         // синий (слишком влажно)
    } else if (LEDType == 2) {  // Температура
        if (dispTemp >= maxTemp) colorCode = 3;      // красный
        else if (dispTemp >= normTemp) colorCode = 15;
        else if (dispTemp >= minTemp) colorCode = 12;
        else colorCode = 48;                         // синий (холодно)
    } else if (LEDType == 3) {  // Осадки / Давление
        if (PRESSURE) {
            // Прогноз дождя (%)
            if (dispRain <= minRain) colorCode = 3;   // красный — сильный дождь
            else if (dispRain <= normRain) colorCode = 15;
            else if (dispRain <= maxRain) colorCode = 48; // синий — улучшение
            else colorCode = 12;                      // зелёный — сухо
        } else {
            // Давление (мм рт.ст.)
            if (dispPres <= minPress) colorCode = 3;
            else if (dispPres <= normPress) colorCode = 15;
            else colorCode = 12;
        }
    }

    // Применяем цвет с учётом яркости
    byte r = (colorCode & 3) ? (LED_MODE ? (255 - LED_ON) : LED_ON) : LED_OFF;
    byte g = ((colorCode >> 2) & 3) ? (LED_MODE ? (255 - LED_ON) : LED_ON) : LED_OFF;
    byte b = ((colorCode >> 4) & 3) ? (LED_MODE ? (255 - LED_ON) : LED_ON) : LED_OFF;

    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
}