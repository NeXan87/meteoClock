#include "predict.h"

#include "bme.h"
#include "config.h"

#define PREDICT_INTERVAL_MS (10 * 60 * 1000)  // 10 минут

static uint32_t pressure_array[6] = {0};
static int cachedRain = 0;
static unsigned long lastUpdate = 0;

bool initPredict() {
    // Инициализация массива текущим давлением
    uint32_t p0 = getBmePressure();  // Pa
    for (byte i = 0; i < 6; i++) {
        pressure_array[i] = p0;
    }
    cachedRain = 0;
    lastUpdate = millis();
    return true;
}

void updatePredict() {
    unsigned long now = millis();
    if (now - lastUpdate < PREDICT_INTERVAL_MS) return;

    // 1. Замер среднего давления за 10 мс (как в оригинале)
    long averPress = 0;
    for (byte i = 0; i < 10; i++) {
        averPress += getBmePressure();  // Pa
        delay(1);
    }
    averPress /= 10;

    // 2. Сдвиг массива
    for (byte i = 0; i < 5; i++) {
        pressure_array[i] = pressure_array[i + 1];
    }
    pressure_array[5] = averPress;

    // 3. Линейная регрессия
    uint32_t sumX = 0, sumY = 0, sumX2 = 0, sumXY = 0;
    for (int i = 0; i < 6; i++) {
        sumX += i;
        sumY += pressure_array[i];
        sumX2 += i * i;
        sumXY += (uint32_t)i * pressure_array[i];
    }

    float a = (6.0f * sumXY - sumX * sumY) / (6.0f * sumX2 - sumX * sumX);
    int delta = (int)(a * 6);  // изменение за 6 шагов

    // 4. Преобразование в %
    cachedRain = map(delta, -250, 250, 100, -100);
    if (cachedRain > 100) cachedRain = 100;
    if (cachedRain < -100) cachedRain = -100;

    lastUpdate = now;
}

int getPredictRain() {
    return cachedRain;
}
