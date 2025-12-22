/*----------------------------------------------------------
    MH-Z19 CO2 sensor  Zero calibration SAMPLE (with ppm + status)
  ----------------------------------------------------------*/

#include <MHZ19_uart.h>

const int rx_pin = 4;  // Serial RX pin (from sensor TX)
const int tx_pin = 5;  // Serial TX pin (to sensor RX)

const int waitingMinutes = 30;  // waiting 30 minutes in clean air

MHZ19_uart mhz19;

/*----------------------------------------------------------
    MH-Z19 CO2 sensor  setup
  ----------------------------------------------------------*/
void setup() {
  Serial.begin(9600);
  mhz19.begin(rx_pin, tx_pin);
  mhz19.setAutoCalibration(false);

  Serial.println("MH-Z19: Starting warm-up (may take 1-3 min)...\n");

  while (mhz19.isWarming()) {
    int ppm = mhz19.getPPM();
    byte status = mhz19.getStatus();
    Serial.print("Warming... Status: ");
    Serial.print(status);
    Serial.print(", CO2: ");
    Serial.print(ppm);
    Serial.println(" ppm");
    delay(1000);
  }

  // Final message after warm-up
  int ppm = mhz19.getPPM();
  byte status = mhz19.getStatus();
  Serial.println("\n✅ Warm-up complete!");
  Serial.print("Status: ");
  Serial.print(status);
  Serial.print(", CO2: ");
  Serial.print(ppm);
  Serial.println(" ppm\n");
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor  loop
  ----------------------------------------------------------*/
long cnt = 0;
const long waitingSeconds = waitingMinutes * 60L;

void loop() {
  if (++cnt % 60 == 0) {
    int ppm = mhz19.getPPM();
    byte status = mhz19.getStatus();
    Serial.print(cnt / 60);
    Serial.print(" min | Status: ");
    Serial.print(status);
    Serial.print(" | CO2: ");
    Serial.print(ppm);
    Serial.println(" ppm");
  } else {
    Serial.print(".");
  }
  delay(1000);

  if (cnt > waitingSeconds) {
    Serial.println("\n--- Performing zero calibration ---");

    // First calibration
    mhz19.calibrateZero();
    delay(2000);
    int ppm1 = mhz19.getPPM();
    byte stat1 = mhz19.getStatus();
    Serial.print("1st calibration done. CO2: ");
    Serial.print(ppm1);
    Serial.print(" ppm, Status: ");
    Serial.println(stat1);

    // Second calibration (optional)
    delay(10000); // wait a bit before second try
    mhz19.calibrateZero();
    delay(2000);
    int ppm2 = mhz19.getPPM();
    byte stat2 = mhz19.getStatus();
    Serial.print("2nd calibration done. CO2: ");
    Serial.print(ppm2);
    Serial.print(" ppm, Status: ");
    Serial.println(stat2);

    // Final readings
    Serial.println("\n--- Post-calibration readings ---");
    for (int i = 0; i < 10; i++) {
      int ppm = mhz19.getPPM();
      byte status = mhz19.getStatus();
      Serial.print("Reading ");
      Serial.print(i + 1);
      Serial.print(": CO2 = ");
      Serial.print(ppm);
      Serial.print(" ppm, Status = ");
      Serial.println(status);
      delay(5000);
    }

    cnt = 0;
    return;
  }
}