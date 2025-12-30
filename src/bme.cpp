#include "bme.h"

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include "config.h"

static Adafruit_BME280 bme;

static bool bmeInitialized = false;
static float cachedTemp = 0.0f;
static float cachedHum = 0.0f;
static float cachedPres = 0.0f;  // в Pa
static float cachedAlt = 0.0f;   // в метрах

bool initBme() {
    if (!bme.begin(&Wire)) {
        bmeInitialized = false;
        return false;
    }
    bme.setSampling(
        Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1,  // температура
        Adafruit_BME280::SAMPLING_X1,  // давление
        Adafruit_BME280::SAMPLING_X1,  // влажность
        Adafruit_BME280::FILTER_OFF);
    bmeInitialized = true;
    updateBme();
    return true;
}

void updateBme() {
    if (!bmeInitialized) return;
    bme.takeForcedMeasurement();
    cachedTemp = bme.readTemperature() + TEMP_OFFSET;
    cachedHum = bme.readHumidity();
    cachedPres = bme.readPressure();                       // Pa
    cachedAlt = bme.readAltitude(SEA_LEVEL_PRESSURE_HPA);  // м
}

float getBmeTemperature() {
    return cachedTemp;
}

float getBmeHumidity() {
    return cachedHum;
}

float getBmePressure() {
    return cachedPres;
}

float getBmeAltitude() {
    return cachedAlt;
}
