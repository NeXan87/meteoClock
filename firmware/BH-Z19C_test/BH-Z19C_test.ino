#include <SoftwareSerial.h>

SoftwareSerial co2Serial(4, 5); // D2=RX (от датчика), D3=TX (к датчику)

void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  Serial.println("MH-Z19C CO2 Reader");
  delay(2000);
}

void loop() {
  // Очистка буфера (на случай мусора)
  while (co2Serial.available()) co2Serial.read();

  // Команда запроса CO2
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  co2Serial.write(cmd, 9);
  delay(120); // важно: дать время на ответ

  byte data[9];
  if (co2Serial.available() >= 9) {
    for (int i = 0; i < 9; i++) {
      data[i] = co2Serial.read();
    }

    // Проверка заголовка
    if (data[0] == 0xFF && data[1] == 0x86) {
      // Расчёт контрольной суммы
      uint8_t checksum = 0;
      for (int i = 1; i <= 7; i++) {
        checksum += data[i];
      }
      checksum = 0xFF - checksum + 1;

      if (checksum == data[8]) {
        uint16_t co2 = (data[2] << 8) | data[3];
        Serial.print("CO2: ");
        Serial.print(co2);
        Serial.println(" ppm");
      } else {
        Serial.print("Checksum error! Recvd: 0x");
        Serial.print(data[8], HEX);
        Serial.print(", Calc: 0x");
        Serial.println(checksum, HEX);
      }
    } else {
      Serial.println("Invalid header");
    }
  } else {
    Serial.println("No full response (timeout)");
  }

  delay(1000);
}