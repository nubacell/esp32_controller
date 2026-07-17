
static void analog_report_one(const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals[i];
  Serial.print(" ");
  //Serial.print(analogSignal);
  //Serial.print("/");
  Serial.print(analogSignal * 0.001, 3);
}

static void analog_report_all(const SensorDeviceData_t & deviceData)
{
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    analog_report_one(deviceData, i);
  }
  Serial.println("");
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
static void SensorDevice_Data_report_one(unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);

#if 1
  //LOG(". sizeof(SensorDeviceData_t)=%zu LGT8F_SPI_DATA_SIZE==%zu", sizeof(SensorDeviceData_t), LGT8F_SPI_DATA_SIZE);
  uint8_t checksum = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.checksum));
  LOG(". checksum=0x%02X", checksum);
#endif
  hex_dump( (const uint8_t *)&deviceData, sizeof(deviceData));

  Serial.print(". . ");
  Serial.print(deviceData.vcc);
  Serial.print("/");
  Serial.print(deviceData.vcc * 0.00125, 4);
  Serial.print(" |");
  analog_report_all(deviceData);
}

static void SensorDevices_Data_report_all()
{
  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    SensorDevice_Data_report_one(i);
  }
}


/*
  uint8_t checksum = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.checksum));
  Serial.print(". checksum="); Serial.println(checksum, HEX);
*/
