#include "co2.h"

#if (CO2_SENSOR == 1)

#include "MHZ19_uart.h"

static MHZ19_uart mhz;
static bool co2Initialized = false;
static int cachedPpm = 0;

bool initCo2() {
    mhz.begin(MHZ_TX, MHZ_RX);
    mhz.setAutoCalibration(false);
    co2Initialized = true;
    updateCo2();
    return true;
}

void updateCo2() {
    if (!co2Initialized) return;
    cachedPpm = mhz.getPPM();
}

int getCo2Ppm() {
    return cachedPpm;
}

int getStatus() {
    if (!co2Initialized) return -1;
    return mhz.getStatus();
}

#endif
