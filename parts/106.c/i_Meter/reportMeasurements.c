
#define I_Meter_Measurements_STRLEN ( \
  CONSTSTRLEN("/TEST/I2C") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" SA=1234567890") \
  + ( \
    + CONSTSTRLEN(" 61440000") \
  ) * I_METER_CHANNELS_COUNT \
  \
  + CONSTSTRLEN("  XX") \
  + CONSTSTRLEN("\n") \
)

void I_Meter::reportMeasurements(
    ::nc::mk::Buffer & b,
    uint32_t now_ms,
    uint32_t test_start_time_ms) const
{
  for (unsigned i = 0; i < I_METER_CHANNELS_COUNT; ++i) {
    const auto v = conversion_results[i];
    // 6.144 / 32768. = 0.1875
    // const auto volts = double(v) * 6.144 / 32768.;
    b.printf(" %8u", v * 1875);
  }
}

void I_Meter::reportMeasurements(
    uint32_t now_ms,
    uint32_t test_start_time_ms) const
{
  char buffer[I_Meter_Measurements_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/TEST/I2C");
  b.printf(" %10u", now_ms - test_start_time_ms);
  b.printf(" SA=%10u", start_time_ms_ - test_start_time_ms);
  iMeter.reportMeasurements(b, now_ms, test_start_time_ms);
  b.printf("  XX\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}
