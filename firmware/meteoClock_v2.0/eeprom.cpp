#include "config.h"
#include "globals.h"
#include "eeprom.h"
#include <EEPROM.h>

void eepromLoadSettings() {
    if (EEPROM.read(0) == EEPROM_SIG_VALUE) {
        MAX_ONDATA = EEPROM.read(2) | (EEPROM.read(3) << 8);
        VIS_ONDATA = EEPROM.read(4) | (EEPROM.read(5) << 8);
        mode0scr   = EEPROM.read(6);
        bigDig     = EEPROM.read(7);
        LED_BRIGHT = EEPROM.read(8);
        LCD_BRIGHT = EEPROM.read(9);
        LEDType    = EEPROM.read(10);
    }
}

void eepromSaveSettings() {
    EEPROM.write(0, EEPROM_SIG_VALUE);
    EEPROM.write(2, MAX_ONDATA & 0xFF);
    EEPROM.write(3, (MAX_ONDATA >> 8) & 0xFF);
    EEPROM.write(4, VIS_ONDATA & 0xFF);
    EEPROM.write(5, (VIS_ONDATA >> 8) & 0xFF);
    EEPROM.write(6, mode0scr);
    EEPROM.write(7, bigDig);
    EEPROM.write(8, LED_BRIGHT);
    EEPROM.write(9, LCD_BRIGHT);
    EEPROM.write(10, LEDType);
}