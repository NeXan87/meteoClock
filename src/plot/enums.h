#pragma once

#include <stdint.h>

namespace Plot {

/**
 * Типы датчиков для графиков
 * Нечётные значения - hour графики, чётные - day
 */
enum class SensorType : uint8_t {
    CO2_Hour        = 1,
    CO2_Day         = 2,
    Humidity_Hour   = 3,
    Humidity_Day    = 4,
    Temperature_Hour = 5,
    Temperature_Day  = 6,
    Pressure_Hour   = 7,
    Pressure_Day    = 8,
    Altitude_Hour   = 9,
    Altitude_Day    = 10
};

/**
 * Пределы значений для разных датчиков
 */
struct SensorRange {
    int minVal;
    int maxVal;
    const char* label;
};

// Вспомогательная функция для получения пределов датчика
inline SensorRange getSensorRange(SensorType type) {
    switch (type) {
        case SensorType::CO2_Hour:
        case SensorType::CO2_Day:
            return {400, 2000, "c "};
        case SensorType::Humidity_Hour:
        case SensorType::Humidity_Day:
            return {0, 100, "h%"};
        case SensorType::Temperature_Hour:
        case SensorType::Temperature_Day:
            return {15, 35, "t"};
        case SensorType::Pressure_Hour:
        case SensorType::Pressure_Day:
            return {720, 760, "p "};
        case SensorType::Altitude_Hour:
        case SensorType::Altitude_Day:
            return {0, 1000, "m "};
        default:
            return {0, 100, ""};
    }
}

}  // namespace Plot
