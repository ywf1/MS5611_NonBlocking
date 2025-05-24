#include "MS5611_NonBlocking.h"

const uint16_t MS5611_NonBlocking::OSR_TIMES[5] = {1, 2, 3, 5, 9};  // ms
const uint8_t MS5611_NonBlocking::D1_COMMANDS[5] = {0x40, 0x42, 0x44, 0x46, 0x48};
const uint8_t MS5611_NonBlocking::D2_COMMANDS[5] = {0x50, 0x52, 0x54, 0x56, 0x58};

MS5611_NonBlocking::MS5611_NonBlocking(uint8_t address, TwoWire* wire)
  : _address(address), _wire(wire), _state(IDLE), _dataReady(false), _osrCode(OSR_4096) {
  _conversionTime = OSR_TIMES[_osrCode];
}

bool MS5611_NonBlocking::begin() {
  _wire->begin();
  if (!checkConnection()) return false;
  reset();
  delay(10);
  readCalibration();
  return true;
}

bool MS5611_NonBlocking::checkConnection() {
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}

void MS5611_NonBlocking::reset() {
  _wire->beginTransmission(_address);
  _wire->write(0x1E);
  _wire->endTransmission();
}

void MS5611_NonBlocking::readCalibration() {
  for (int i = 1; i <= 6; i++) {
    _wire->beginTransmission(_address);
    _wire->write(0xA0 + i * 2);
    _wire->endTransmission();
    _wire->requestFrom(_address, 2);
    _coeff[i] = (_wire->read() << 8) | _wire->read();
  }
}

void MS5611_NonBlocking::setOSR(OSR osr) {
  _osrCode = osr;
  _conversionTime = OSR_TIMES[_osrCode];
}

void MS5611_NonBlocking::update() {
  unsigned long now = millis();

  switch (_state) {
    case IDLE:
      startTemperatureConversion();
      _state = WAIT_TEMP;
      _lastRequestTime = now;
      break;

    case WAIT_TEMP:
      if (now - _lastRequestTime >= _conversionTime) {
        readADC(_rawTemp);
        startPressureConversion();
        _state = WAIT_PRESS;
        _lastRequestTime = now;
      }
      break;

    case WAIT_PRESS:
      if (now - _lastRequestTime >= _conversionTime) {
        readADC(_rawPress);
        computeCompensation();
        _dataReady = true;
        _state = IDLE;
      }
      break;
  }
}

void MS5611_NonBlocking::startTemperatureConversion() {
  _wire->beginTransmission(_address);
  _wire->write(D2_COMMANDS[_osrCode]);
  _wire->endTransmission();
}

void MS5611_NonBlocking::startPressureConversion() {
  _wire->beginTransmission(_address);
  _wire->write(D1_COMMANDS[_osrCode]);
  _wire->endTransmission();
}

void MS5611_NonBlocking::readADC(uint32_t& result) {
  _wire->beginTransmission(_address);
  _wire->write(0x00);
  _wire->endTransmission();
  _wire->requestFrom(_address, 3);
  result = ((uint32_t)_wire->read() << 16) | ((uint32_t)_wire->read() << 8) | _wire->read();
}

void MS5611_NonBlocking::computeCompensation() {
  int32_t dT = _rawTemp - ((uint32_t)_coeff[5] << 8);
  int32_t TEMP = 2000 + ((int64_t)dT * _coeff[6]) / 8388608;

  int64_t OFF = ((int64_t)_coeff[2] << 16) + ((int64_t)_coeff[4] * dT) / 128;
  int64_t SENS = ((int64_t)_coeff[1] << 15) + ((int64_t)_coeff[3] * dT) / 256;

  int32_t T2 = 0;
  int64_t OFF2 = 0, SENS2 = 0;

  if (TEMP < 2000) {
    T2 = (dT * dT) >> 31;
    OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) >> 1;
    SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) >> 2;

    if (TEMP < -1500) {
      OFF2 += 7 * ((TEMP + 1500) * (TEMP + 1500));
      SENS2 += 11 * ((TEMP + 1500) * (TEMP + 1500)) >> 1;
    }

    TEMP -= T2;
    OFF -= OFF2;
    SENS -= SENS2;
  }

  int32_t P = (((int64_t)_rawPress * SENS) / 2097152 - OFF) / 32768;

  _temperature = TEMP / 100.0;
  _pressure = P / 100.0;
}

bool MS5611_NonBlocking::dataReady() {
  if (_dataReady) {
    _dataReady = false;
    return true;
  }
  return false;
}

float MS5611_NonBlocking::getTemperature() {
  return _temperature;
}

float MS5611_NonBlocking::getPressure() {
  return _pressure;
}

float MS5611_NonBlocking::getAltitude(float seaLevelPressure) {
  return 44330.0 * (1.0 - pow(_pressure / seaLevelPressure, 0.1903));
}