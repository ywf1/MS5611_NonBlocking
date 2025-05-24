//Yahya Farag
//5/23/2025
#include "MS5611_NonBlocking.h"
#include <Wire.h>

//alternate adress is 0x76, TwoWire Compatible for arduino cores with multiple I2C peripherals
MS5611_NonBlocking ms5611(0x77, &Wire);

void setup() {
  //delay before serial connection
  delay(3000);

  Serial.begin(9600);

  if (!ms5611.begin()) {
    Serial.println("MS5611 not detected!");
    while (1);
  }

  // ============================================================================
  // Oversampling Ratio (OSR) Settings:
  // ms5611.setOSR(MS5611_NonBlocking::OSR_256);   // ~0.54 ms → ~1850 Hz max
  // ms5611.setOSR(MS5611_NonBlocking::OSR_512);   // ~1.06 ms → ~940 Hz
  // ms5611.setOSR(MS5611_NonBlocking::OSR_1024);  // ~2.08 ms → ~480 Hz
  // ms5611.setOSR(MS5611_NonBlocking::OSR_2048);  // ~4.13 ms → ~240 Hz
  // ms5611.setOSR(MS5611_NonBlocking::OSR_4096);  // ~8.22 ms → ~120 Hz (default)
  // ============================================================================

  ms5611.setOSR(MS5611_NonBlocking::OSR_4096);  // Use highest resolution
}

void loop() {
  //start conversion
  ms5611.update();

  //when conversion is ready get newest data
  if (ms5611.dataReady()) {
    float temp = ms5611.getTemperature();
    float press = ms5611.getPressure();
    float alt = ms5611.getAltitude();
  }

  //print out values
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" °C, Pressure: ");
  Serial.print(press);
  Serial.print(" mbar, Altitude: ");
  Serial.print(alt);
  Serial.println(" m");
}