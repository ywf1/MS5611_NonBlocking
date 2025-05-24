#ifndef MS5611_NONBLOCKING_H
#define MS5611_NONBLOCKING_H

#include <Arduino.h>
#include <Wire.h>

class MS5611_NonBlocking {
public:
  enum OSR {
    OSR_256  = 0,
    OSR_512  = 1,
    OSR_1024 = 2,
    OSR_2048 = 3,
    OSR_4096 = 4
  };

  enum State {
    IDLE, START_TEMP, WAIT_TEMP, START_PRESS, WAIT_PRESS
  };

  MS5611_NonBlocking(uint8_t address = 0x77, TwoWire* wire = &Wire);

  bool begin();                          // Returns false if sensor not found
  void update();                         // Call frequently
  bool dataReady();                      // True when new data is available
  float getTemperature();               // °C
  float getPressure();                  // mbar
  float getAltitude(float seaLevelPressure = 1013.25);  // meters
  void setOSR(OSR osr);                  // Set oversampling rate

private:
  void reset();
  bool checkConnection();
  void readCalibration();
  void startTemperatureConversion();
  void startPressureConversion();
  void readADC(uint32_t& result);
  void computeCompensation();

  uint8_t _address;
  TwoWire* _wire;
  State _state;
  unsigned long _lastRequestTime;
  uint16_t _conversionTime;  // ms
  uint8_t _osrCode;          // Command index

  uint16_t _coeff[7];
  uint32_t _rawTemp, _rawPress;
  float _temperature, _pressure;
  bool _dataReady;

  static const uint16_t OSR_TIMES[5];
  static const uint8_t D1_COMMANDS[5];
  static const uint8_t D2_COMMANDS[5];
};

#endif