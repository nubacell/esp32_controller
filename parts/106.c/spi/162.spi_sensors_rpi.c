
namespace rpi {

static size_t analog_report_one(
    ::nc::mk::Buffer & b,
    const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals.all[i];
  return b.printf(" %4u", analogSignal);
}

static size_t analog_report_all(
    ::nc::mk::Buffer & b,
    const SensorDeviceData_t & deviceData)
{
  size_t used = 0;
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    used += ::rpi::analog_report_one(b, deviceData, i);
  }
  return used;
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
static size_t SensorDevice_Data_report_one(
    ::nc::mk::Buffer & b,
    unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);
#if 1
  size_t used = b.printf("  %6u", deviceData.vcc * 125);
#else
  size_t used = b.printf("  %4u", deviceData.vcc);
#endif
  used += ::rpi::analog_report_all(b, deviceData);
  return used;
}

/*
    now_ms
    spiDevice.transfer_start_time_ms_
    spiDevice.transfer_end_time_ms_
*/
#define SensorDevices_Data_STRLEN ( \
  CONSTSTRLEN("/TEST/SPI") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" SA=1234567890") \
  + CONSTSTRLEN(" EA=1234567890") \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 1234") * ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSOR_DEVICES_COUNT \
  \
  + CONSTSTRLEN("  XX") \
  + CONSTSTRLEN("\n") \
)

void SensorDevices_Data_report_all(
    ::nc::mk::Buffer & b,
    uint32_t now_ms,
    uint32_t test_start_time_ms)
{
  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    ::rpi::SensorDevice_Data_report_one(b, i);
  }
}

static void SensorDevices_Data_report_all(
    uint32_t now_ms,
    uint32_t test_start_time_ms)
{
  static char buffer[SensorDevices_Data_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/TEST/SPI");
  b.printf(" %10u", now_ms - test_start_time_ms);
  b.printf(" SA=%10u", spiDevice.transfer_start_time_ms_ - test_start_time_ms);
  b.printf(" EA=%10u", spiDevice.transfer_end_time_ms_ - test_start_time_ms);
  rpi::SensorDevices_Data_report_all(b, now_ms, test_start_time_ms);
  b.printf("  XX\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}

} // namespace rpi
