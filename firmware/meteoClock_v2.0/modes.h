#pragma once

#include <Arduino.h>

// Внешние переменные (определены в globals.cpp)
extern byte mode;
extern byte podMode;
extern byte mode0scr;
extern boolean bigDig;

// Функции
void modesInit();
void modesTick();
void redrawPlot();
void drawSensors();
void drawData();