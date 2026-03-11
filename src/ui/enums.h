#pragma once

#include <stdint.h>

namespace UI {

/**
 * Основные режимы работы UI
 */
enum class Mode : uint8_t {
    Clock       = 0,    // часы
    Sensor1     = 1,    // графики датчиков
    Sensor2     = 2,
    Sensor3     = 3,
    Sensor4     = 4,
    Sensor5     = 5,
    Sensor6     = 6,
    Sensor7     = 7,
    Sensor8     = 8,
    Sensor9     = 9,
    Sensor10    = 10,
    // сервисные режимы (240+)
    LED_Mode    = 252,  // выбор режима LED
    LED_Bright  = 253,  // яркость LED (presets)
    LED_Manual  = 254,  // ручной режим LED
    Menu        = 255   // главное меню
};

/**
 * Подрежимы для LED_Mode (выбор режима светодиода)
 */
enum class LEDMode : uint8_t {
    Off     = 0,
    Auto    = 1,
    Warning = 2,
    Alarm   = 3,
    Manual  = 4
};

/**
 * Режимы привязки LED к датчикам
 */
enum class LEDBindMode : uint8_t {
    CO2         = 0,
    Humidity    = 1,
    Temperature = 2,
    Rain        = 3
};

/**
 * Статусы тревог для LED индикации
 */
enum class AlertStatus : uint8_t {
    Normal      = 0,    // зелёный
    Warning     = 1,    // жёлтый
    Critical    = 2,    // красный
    Blinking    = 3     // мигание
};

/**
 * Режимы отображения на главном экране (mode0scr)
 */
enum class MainDisplayMode : uint8_t {
    Time        = 0,    // часы крупно
    CO2         = 1,    // CO2 крупно
    Temperature = 2,    // температура крупно
    Pressure    = 3,    // давление крупно
    Humidity    = 4,    // влажность крупно
    Altitude    = 5     // высота крупно
};

/**
 * Подрежимы для LED_Bright и LED_Manual (яркость/уровень)
 * 0-11 для presets, 0-15 для manual
 */
enum class LEDLevel : uint8_t {
    Level0  = 0,
    Level1  = 1,
    Level2  = 2,
    Level3  = 3,
    Level4  = 4,
    Level5  = 5,
    Level6  = 6,
    Level7  = 7,
    Level8  = 8,
    Level9  = 9,
    Level10 = 10,
    Level11 = 11,
    Level12 = 12,
    Level13 = 13,
    Level14 = 14,
    Level15 = 15
};

/**
 * Подрежимы главного меню (Menu)
 */
enum class MenuMode : uint8_t {
    Exit        = 1,    // выход
    Save        = 2,    // сохранить настройки
    // 3-5 зарезервировано
    VisSensor1  = 6,    // отображение датчиков (битовая маска)
    VisSensor12 = 17    // последний датчик
};

}  // namespace UI
