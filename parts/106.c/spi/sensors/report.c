
void analog_report_one(const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals.all[i];
  Serial.print(" ");
  //Serial.print(analogSignal);
  //Serial.print("/");
  Serial.print(analogSignal * 0.001, 3);
}

void analog_report_all(const SensorDeviceData_t & deviceData)
{
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    analog_report_one(deviceData, i);
  }
  Serial.println("");
}

// byte ::nc::spi::buffers::rx[SPI_SENSORS_COUNT][SPI_BUFFER_SIZE];
void SensorDevice_Data_report_one(unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&::nc::spi::buffers::rx[i][0]);

#if 1
  //LOG(". sizeof(SensorDeviceData_t)=%zu LGT8F_SPI_DATA_SIZE==%zu", sizeof(SensorDeviceData_t), LGT8F_SPI_DATA_SIZE);
#if 1
  const auto crc = ::nc::spi::sensors::crc(&deviceData);
#else
  uint8_t crc = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.crc));
#endif
  LOG(". crc=0x%02X %s", crc, (deviceData.crc == crc) ? "OK" : "NG");
#endif
  hex_dump( (const uint8_t *)&deviceData, sizeof(deviceData));

  Serial.print(". . ");
  Serial.print(deviceData.vcc);
  Serial.print("/");
  Serial.print(deviceData.vcc * 0.00125, 4);
  Serial.print(" |");
  analog_report_all(deviceData);
}

void SensorDevices_Data_report_all()
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    SensorDevice_Data_report_one(i);
  }
}


/*
  uint8_t crc = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.crc));
  Serial.print(". crc="); Serial.println(crc, HEX);
*/
