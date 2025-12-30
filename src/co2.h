#pragma once

#include <Arduino.h>

#include "config.h"

#if (CO2_SENSOR == 1)
bool initCo2();
void updateCo2();
int getCo2Ppm();
#else
bool initCo2();
void updateCo2();
int getCo2Ppm();
int getStatus();
#endif
