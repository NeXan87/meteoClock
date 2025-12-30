#pragma once

#include <Arduino.h>

void initLed();
void updateLed();
void setLedColor(byte color);
void setLedType(byte type);
void setLedBrightness(byte brightness);
byte getLedType();
byte getLedBrightness();
void notifyAmbientLight(bool isDark);
