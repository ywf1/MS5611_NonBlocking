# MS5611_NonBlocking

Developed this library after noticing several other libraries using the MS5611 barometric pressure sensor would block the main execution loop due to how this chip handles conversion. This library is very low profile and compatible with all arduino archetectures.

A non-blocking driver for the MS5611 barometric pressure sensor.

- Configurable oversampling resolution (OSR)
- Second-order temperature compensation
- Altitude calculation from pressure
- I2C bus selection and address

For hardware developers I highley reccomend moving to the Bosch BMP581. It is a much newer, cheaper, barometer with higher data rate at much better resolution.

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

## License

MIT
