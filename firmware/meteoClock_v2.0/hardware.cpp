// hardware.cpp
#include "config.h"
#include "hardware.h"

#if (DISPLAY_TYPE == 1)
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);
#else
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 16, 2);
#endif

GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);