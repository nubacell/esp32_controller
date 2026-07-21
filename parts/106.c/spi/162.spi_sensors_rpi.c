
namespace rpi {

static void analog_report_one(const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals[i];
  Serial.print(" ");
  //Serial.print(analogSignal);
  Serial.print(analogSignal);
  //Serial.print("/");
  //Serial.print(analogSignal * 0.001, 3);
}

static void analog_report_all(const SensorDeviceData_t & deviceData)
{
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    ::rpi::analog_report_one(deviceData, i);
  }
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
static void SensorDevice_Data_report_one(unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);

  Serial.print("  ");
  Serial.print(deviceData.vcc * 125);
  //Serial.print(deviceData.vcc);
  //Serial.print("/");
  //Serial.print(deviceData.vcc * 5 / 4);
  //Serial.print("/");
  //Serial.print(deviceData.vcc * 0.00125, 4);
  //Serial.print(" |");
  ::rpi::analog_report_all(deviceData);
}

static void SensorDevices_Data_report_all()
{
  const auto us1 = esp_timer_get_time(); // uint64_t
  const auto availableForWrite1 = cn_uartConfig.availableForWrite();
  Serial.print("/TEST/SPI");
  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    ::rpi::SensorDevice_Data_report_one(i);
  }
  Serial.println("");
  const auto availableForWrite2 = cn_uartConfig.availableForWrite();
  const auto us2 = esp_timer_get_time(); // uint64_t
  LOG("availableForWrite2=%u - availableForWrite1=%u =%u",
      availableForWrite2, availableForWrite1, availableForWrite2 - availableForWrite1);
  LOG("us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, us2, us1, us2 - us1);
}

} // namespace rpi
