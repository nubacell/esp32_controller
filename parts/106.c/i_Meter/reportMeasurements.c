
#define I_Meter_Measurements_STRLEN ( \
  CONSTSTRLEN("/TEST/I2C") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + ( \
    + CONSTSTRLEN(" 12345") \
    + CONSTSTRLEN(" 61440000") \
    + CONSTSTRLEN(" 6.1440") \
  ) * I_METER_CHANNELS_COUNT \
  + CONSTSTRLEN(" xx") \
  + CONSTSTRLEN("\n") \
)

void I_Meter::reportMeasurements(
    uint32_t now_ms,
    uint32_t test_start_time_ms) const
{
  char buffer[I_Meter_Measurements_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

#if 1
  const auto us1 = esp_timer_get_time(); // uint64_t
#endif

  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/I2C");

  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        now_ms - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        start_time_ms_ - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        finish_time_ms_ - test_start_time_ms);
  }

  for (unsigned i = 0; i < I_METER_CHANNELS_COUNT && buffer_size > used; ++i) {
    const size_t left = buffer_size - used;
    const auto v = conversion_results[i];
    // 6.144 / 32768. = 0.1875
    const auto volts = double(v) * 6.144 / 32768.;
    used += safe_snprintf(&buffer[used], left, " %5u %8u %6.4f", v, v * 1875, volts);
  }

  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " xx\n");
  }

  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);

#if 1
  const auto us2 = esp_timer_get_time(); // uint64_t
  LOG("us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, us2, us1, us2 - us1);
#endif
}
