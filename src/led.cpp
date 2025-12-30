#include "led.h"

#include "bme.h"
#include "co2.h"
#include "config.h"
#include "rtc.h"

// Внутренние переменные (инкапсулированы)
static byte ledType = 0;     //  при отсутствии сохранения в EEPROM: привязка индикатора к датчикам:
                             //  0 - СО2, 1 - Влажность, 2 - Температура, 3 - Осадки
static byte ledBright = 10;  // при отсутствии сохранения в EEPROM: яркость светодиода СО2 (0 - 10)
                             // (коэффициент настраиваемой яркости индикатора по умолчанию, если нет
                             // сохранения и не автоматическая регулировка (с)НР)
static bool ledInitialized = false;
static bool ambientIsDark = false;

float dispTemp = getBmeTemperature();
float dispHum = getBmeHumidity();
float dispPres = getBmePressure() * PA_TO_MMHG;
int dispCO2 = getCo2Ppm();
extern int dispRain;

void initLed() {
    pinMode(LED_COM, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_COM, LED_MODE);  // общий анод/катод

    ledInitialized = true;
    updateLed();
}

// Вспомогательная функция: вычисление яркости
static byte computeLedOn() {
    if (ledBright == 11) {
        // Автояркость — пока не реализована (в main.cpp она в checkBrightness)
        // Для простоты используем MAX
        return (LED_MODE == 0) ? LED_BRIGHT_MAX : (255 - LED_BRIGHT_MAX);
    } else {
        // Ручная яркость: масштабируем квадратично (как в оригинале)
        byte level = (255 * ledBright * ledBright) / 100;
        return (LED_MODE == 0) ? level : (255 - level);
    }
}

// Основная логика индикации
void updateLed() {
    if (!ledInitialized) return;

    byte color = 12;  // зелёный по умолчанию

    // Определяем цвет в зависимости от типа индикации и текущих значений
    if (ledType == 0) {                           // CO2
        if (dispCO2 >= maxCO2) color = 3;         // красный
        else if (dispCO2 >= normCO2) color = 48;  // синий
        else color = 12;                          // зелёный
        if (dispCO2 >= blinkLEDCO2) {
            setLedColor(0);
            return;
        }

    } else if (ledType == 1) {  // Влажность
        if (dispHum <= minHum) color = 3;
        else if (dispHum <= normHum) color = 48;
        else if (dispHum <= maxHum) color = 12;
        else color = 48;  // выше maxHum — синий
        if (dispHum <= blinkLEDHum) {
            setLedColor(0);
            return;
        }

    } else if (ledType == 2) {  // Температура
        if (dispTemp >= maxTemp) color = 3;
        else if (dispTemp >= normTemp) color = 48;
        else color = 12;
        if (dispTemp >= blinkLEDTemp) {
            setLedColor(0);
            return;
        }

    } else if (ledType == 3) {  // Осадки
        if (dispRain <= minRain) color = 3;
        else if (dispRain <= normRain) color = 48;
        else color = 12;

    } else if (ledType == 4) {  // Давление
        if (dispPres <= minPress) color = 3;
        else if (dispPres <= normPress) color = 48;
        else color = 12;
    }

    setLedColor(color);
}

// Низкоуровневая установка цвета
void setLedColor(byte color) {
    byte on = computeLedOn();
    byte off = (LED_MODE == 0) ? 0 : 255;

    analogWrite(LED_R, off + (on - off) * (3 - (color & 3)) / 3);
    analogWrite(LED_G, off + (on - off) * (3 - ((color & 12) >> 2)) / 3);
    analogWrite(LED_B, off + (on - off) * (3 - ((color & 48) >> 4)) / 3);
}

void notifyAmbientLight(bool isDark) {
    if (ambientIsDark != isDark) {
        ambientIsDark = isDark;
        if (ledBright == 11) {
            updateLed();  // перерисовать с новой авто-яркостью
        }
    }
}

// ------------------------------------------------------------
// Геттеры/сеттеры для связи с меню
void setLedType(byte type) { ledType = type; }
void setLedBrightness(byte brightness) { ledBright = brightness; }
byte getLedType() { return ledType; }
byte getLedBrightness() { return ledBright; }
