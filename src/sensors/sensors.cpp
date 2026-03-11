#include "sensors/sensors.h"

#include <Adafruit_BME280.h>
#include <Arduino.h>

#include "config.h"
#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>
#endif

namespace Sensors {

static Adafruit_BME280 bme;
#if (CO2_SENSOR == 1)
static MHZ19_uart mhz19;
#endif

// внутреннее состояние
static float temp = 0;
static uint8_t hum = 0;
static int pres = 0;
static int co2 = 0;
static float alt = 0;
static int rain = 0;  // вычисляется в PlotManager

void init() {
    Wire.begin();
    bme.begin();
#if (CO2_SENSOR == 1)
    mhz19.begin(MHZ_RX, MHZ_TX);
#endif
}

void update() {
    bme.takeForcedMeasurement();
    temp = bme.readTemperature();
    hum = static_cast<uint8_t>(bme.readHumidity());
    alt = ((float)alt * 1 + bme.readAltitude(SEALEVELPRESSURE_HPA)) / 2.0f;  // сглаживание
    pres = static_cast<int>(bme.readPressure() * 0.00750062);
#if (CO2_SENSOR == 1)
    co2 = mhz19.getPPM();
#else
    co2 = 0;
#endif
}

float getTemp() { return temp; }
uint8_t getHumidity() { return hum; }
int getPres() { return pres; }
int getCO2() { return co2; }
float getAlt() { return alt; }
int getRain() { return rain; }

void setRain(int r) { rain = r; }

}  // namespace Sensors
