
void analog_report_one(const ::nc::lgt::SensorData & sensorData, unsigned i)
{
  const auto analogSignal = sensorData.analogSignals.all[i];
  Serial.print(" ");
  Serial.print(analogSignal * 0.001, 3);
}

void analog_report_all(const ::nc::lgt::SensorData & sensorData)
{
  for (unsigned i = 0; i < ::nc::lgt::ALL_ANALOG_SIGNALS_COUNT; ++i) {
    analog_report_one(sensorData, i);
  }
  Serial.println("");
}

void SensorDevice_Data_report_one(unsigned i)
{
  const auto & sensor = ::nc::spi::sensors.sensors_[i];
  const auto & sensorData = sensor.raw.data;

#if 1
  const auto crc = sensor.raw.crc();
  LOG(". crc=0x%02X %s", crc, (sensorData.crc == crc) ? "OK" : "NG");
#endif
  hex_dump( (const uint8_t *)&sensorData, sizeof(sensorData));

  Serial.print(". . ");
  Serial.print(sensorData.vcc);
  Serial.print("/");
  Serial.print(sensorData.vcc * 0.00125, 4);
  Serial.print(" |");
  analog_report_all(sensorData);
}

void spi_report_data()
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    SensorDevice_Data_report_one(i);
  }
}
