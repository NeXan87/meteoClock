#include "drivers/mhz19.h"

#if (CO2_SENSOR == 1)

#include <Arduino.h>

namespace MHZ19 {

static MHZ19_uart mhz19;
static int co2Value = 0;
static unsigned long lastUpdate = 0;

void init() {
    mhz19.begin(MHZ_TX, MHZ_RX);
    co2Value = mhz19.getPPM();
    lastUpdate = millis();
}

int getCO2() {
    if (millis() - lastUpdate >= SENS_TIME) {
        co2Value = mhz19.getPPM();
        lastUpdate = millis();
    }
    return co2Value;
}

}  // namespace MHZ19

#endif  // CO2_SENSOR == 1
