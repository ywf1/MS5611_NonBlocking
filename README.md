# MS5611_NonBlocking

Developed this library after noticing several other libraries using the MS5611 barometric pressure sensor would block the main execution loop due to how this chip handles conversion. This library is very low profile and compatible with all arduino archetectures.

A non-blocking driver for the MS5611 barometric pressure sensor.

- Configurable oversampling resolution (OSR)
- Second-order temperature compensation
- Altitude calculation from pressure
- I2C bus selection and address

For hardware developers I recommend moving to the Bosch BMP581. It is a much newer and cheaper barometer with a higher data rate at much better resolution.

## Example Usage

```cpp
#include "MS5611_NonBlocking.h"

MS5611_NonBlocking ms5611(0x77, &Wire);

void setup() {
  delay(3000);
  
  Serial.begin(9600);

  if (!ms5611.begin()) {
    Serial.println("Sensor not detected!");
    while (1);
  }
  ms5611.setOSR(MS5611_NonBlocking::OSR_4096);
}

void loop() {
  ms5611.update();
  if (ms5611.dataReady()) {
    Serial.print("Temp: ");
    Serial.print(ms5611.getTemperature());
    Serial.print(" °C, Pressure: ");
    Serial.print(ms5611.getPressure());
    Serial.print(" mbar, Altitude: ");
    Serial.print(ms5611.getAltitude());
    Serial.println(" m");
  }
}
```

## Moving Average
These sensors tend to be very noisy, in order to get usable data it is best to apply a N window (N = 10 to 30) moving average to smooth out the readings.

```cpp
#include "MS5611_NonBlocking.h"
#include <Wire.h>

MS5611_NonBlocking ms5611(0x77, &Wire);

const int avgWindow = 10;
float pressureBuffer[avgWindow];
int bufferIndex = 0;
int bufferCount = 0;

void setup() {
  Serial.begin(9600);
  if (!ms5611.begin()) {
    Serial.println("Sensor not found!");
    while (1);
  }
  ms5611.setOSR(MS5611_NonBlocking::OSR_1024);  // balance speed and resolution
}

void loop() {
  ms5611.update();

  if (ms5611.dataReady()) {
    float pressure = ms5611.getPressure();

    // Insert pressure into buffer
    pressureBuffer[bufferIndex] = pressure;
    bufferIndex = (bufferIndex + 1) % avgWindow;
    if (bufferCount < avgWindow) bufferCount++;

    // Compute moving average
    float sum = 0;
    for (int i = 0; i < bufferCount; i++) sum += pressureBuffer[i];
    float avgPressure = sum / bufferCount;

    Serial.print("Raw Pressure: ");
    Serial.print(pressure);
    Serial.print(" mbar, Smoothed: ");
    Serial.print(avgPressure);
    Serial.println(" mbar");
  }
}

```

## License

MIT
