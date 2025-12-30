#pragma once

#include <Arduino.h>

bool initRtc();
void updateRtc();

uint8_t getRtcHours();
uint8_t getRtcMinutes();
uint8_t getRtcSeconds();
uint8_t getRtcDay();
uint8_t getRtcMonth();
uint16_t getRtcYear();
uint8_t getRtcDayOfWeek();
