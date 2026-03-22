#include "drivers/bme280.h"

#include <Adafruit_BME280.h>
#include <Arduino.h>

#include "config.h"

namespace BME280 {

static Adafruit_BME280 bme;

static float temp = 0;
static float hum = 0;
static float pres = 0;
static int rain = 0;

void init() {
    Wire.begin();
    bme.begin();
    update();
}

void update() {
    bme.takeForcedMeasurement();
    temp = bme.readTemperature();
    hum = bme.readHumidity();
    pres = bme.readPressure() * 0.00750062;
}

float getTemp() { return temp; }
float getHumidity() { return hum; }
float getPres() { return pres; }
int getRain() { return rain; }
void setRain(int r) { rain = r; }

}  // namespace BME280
