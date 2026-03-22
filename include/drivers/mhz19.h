#pragma once

#include "config.h"

#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>

namespace MHZ19 {

void init();
int getCO2();

}  // namespace MHZ19

#endif  // CO2_SENSOR == 1
