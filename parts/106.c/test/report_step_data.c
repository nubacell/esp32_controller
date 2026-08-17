
#define TEST_STEP_DATA_STRLEN ( \
  CONSTSTRLEN("/TEST/DATA") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN("  |") \
  \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + ( \
    + CONSTSTRLEN(" 12345") \
    + CONSTSTRLEN(" 61440000") \
    + CONSTSTRLEN(" 6.1440") \
  ) * I_METER_CHANNELS_COUNT \
  \
  + CONSTSTRLEN("  |") \
  \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 1234") * ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSOR_DEVICES_COUNT \
  \
  + CONSTSTRLEN("  XX") \
  + CONSTSTRLEN("\n") \
)

void report_step_data(
    uint32_t now_ms,
    uint32_t test_start_time_ms)
{
  char buffer[TEST_STEP_DATA_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/TEST/DATA");
  b.printf(" %10u", now_ms - test_start_time_ms);

  b.printf("  |");
  iMeter.reportMeasurements(b, now_ms, test_start_time_ms);
  b.printf("  |");
  rpi::SensorDevices_Data_report_all(b, now_ms, test_start_time_ms);

  b.printf("  XX\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}
