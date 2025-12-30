#pragma once

#include <Arduino.h>

#include "config.h"

#if (CO2_SENSOR == 1)
bool initCo2();
void updateCo2();
int getCo2Ppm();
int getStatus();
#else
static inline bool initCo2() { return true; }
static inline void updateCo2() {}
static inline int getCo2Ppm() { return 0; }
static inline int getStatus() { return -1; }
#endif
