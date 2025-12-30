#include "bme.h"

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include "config.h"

static Adafruit_BME280 bme;

static bool bmeInitialized = false;

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
}

float getBmeTemperature() {
    return bme.readTemperature() + TEMP_OFFSET;
}

float getBmeHumidity() {
    return bme.readHumidity();
}

float getBmePressure() {
    return bme.readPressure();
}

float getBmeAltitude() {
    return bme.readAltitude(SEA_LEVEL_PRESSURE_HPA);
}
