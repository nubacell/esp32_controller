
namespace rpi {

static size_t analog_report_one(
    char * buffer, size_t buffer_size,
    const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals[i];
  return safe_snprintf(buffer, buffer_size, " %4u", analogSignal);
}

static size_t analog_report_all(
    char * buffer, size_t buffer_size,
    const SensorDeviceData_t & deviceData)
{
  size_t used = 0;
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    if (buffer_size > used) {
      const size_t left = buffer_size - used;
      used += ::rpi::analog_report_one(&buffer[used], left, deviceData, i);
    }
  }
  return used;
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
static size_t SensorDevice_Data_report_one(
    char * buffer, size_t buffer_size,
    unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);

#if 1
  size_t used = safe_snprintf(buffer, buffer_size, "  %6u", deviceData.vcc * 125);
#else
  size_t used = safe_snprintf(buffer, buffer_size, "  %4u", deviceData.vcc);
#endif
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += ::rpi::analog_report_all(&buffer[used], left, deviceData);
  }
  return used;
}

/*
    now_ms
    spiDevice.transfer_start_time_ms_
    spiDevice.transfer_finish_time_ms_
*/
#define SensorDevices_Data_STRLEN ( \
  CONSTSTRLEN("/TEST/SPI") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 1234") * ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSOR_DEVICES_COUNT \
  + CONSTSTRLEN("  FF\n") \
)

static void SensorDevices_Data_report_all(
    uint32_t now_ms,
    uint32_t test_start_time_ms)
{
  static char buffer[SensorDevices_Data_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

#if 0
  const auto us1 = esp_timer_get_time(); // uint64_t
  const auto availableForWrite1 = cn_uartConfig.availableForWrite();
#endif

  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/SPI");
#if 0
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, "  %10" PRIu64, us1);
  }
#endif

  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        now_ms - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        spiDevice.transfer_start_time_ms_ - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        spiDevice.transfer_finish_time_ms_ - test_start_time_ms);
  }

  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    if (buffer_size > used) {
      const size_t left = buffer_size - used;
      used += ::rpi::SensorDevice_Data_report_one(&buffer[used], left, i);
    }
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, "  XX\n");
  }
  if (used < buffer_size) {
    Serial.write(buffer, used);
    //LOG("buffer_size=%zu required=%zu", buffer_size, used + 1);
  } else {
    buffer[buffer_size - 2] = '\n';
    buffer[buffer_size - 1] = '\0';
    Serial.write(buffer, buffer_size - 1);
    LOG("ERROR: buffer overflow (buffer_size=%zu required=%zu)", buffer_size, used + 1);
  }
#if 0
  const auto availableForWrite2 = cn_uartConfig.availableForWrite();
  const auto us2 = esp_timer_get_time(); // uint64_t
  LOG("availableForWrite1=%u - availableForWrite2=%u =%i",
      availableForWrite1, availableForWrite2,
      int(availableForWrite1) - int(availableForWrite2));
  LOG("us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, us2, us1, us2 - us1);
#endif
}

} // namespace rpi
