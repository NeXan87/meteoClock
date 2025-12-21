#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Укажите I2C-адрес: 0x76 или 0x77 (проверьте ваш модуль)
#define BME280_ADDRESS 0x76

Adafruit_BME280 bme; // Использует Wire по умолчанию

void setup() {
  Serial.begin(9600);
  while (!Serial); // Ждём открытия монитора (для Leonardo/Micro)

  Serial.println("BME280 Sensor Test");
  Serial.println("===================");

  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    Serial.println("Try address 0x76 or 0x77.");
    while (1); // зависаем
  }

  Serial.println("BME280 Found!");
  Serial.println();
}

void loop() {
  // Чтение всех значений
  float temperature = bme.readTemperature();     // °C
  float humidity = bme.readHumidity();           // %
  float pressure = bme.readPressure() / 100.0F;  // гПа (мбар)

  // Опционально: расчёт высоты
  float altitude = bme.readAltitude(1013.25);    // высота над уровнем моря (при давлении 1013.25 гПа)

  // Вывод в монитор
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure    = ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(altitude);
  Serial.println(" m");

  Serial.println("-------------------");
  delay(1000);
}