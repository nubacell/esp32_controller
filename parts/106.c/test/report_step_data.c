
#define TEST_STEP_DATA_STRLEN ( \
  CONSTSTRLEN("/TEST/DATA") \
  \
  + CONSTSTRLEN(" 99") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" 99999") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + ( \
    + CONSTSTRLEN(" 61440000") \
  ) * I_METER_CHANNELS_COUNT \
  \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 1234") * ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSORS_COUNT \
  \
  + CONSTSTRLEN("  XX") \
  + CONSTSTRLEN("\n") \
)

void Test::report_step_data(
    uint32_t now_ms,
    uint32_t test_start_time_ms) const
{
  char buffer[TEST_STEP_DATA_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

//test_start_time_ms = 0; // FIXME: remove this line

  b.printf("/TEST/DATA");

  // SP SA EP
//b.printf(" step:");
  b.printf(" %2u", step_.count_);
  b.printf(" %10u", step_.start_time_planned_ms_ - test_start_time_ms);
  b.printf(" %10u", step_.start_time_actual_ms_ - test_start_time_ms);
  b.printf(" %10u", step_.end_time_planned_ms_ - test_start_time_ms);

  // SP SA EP
//b.printf(" sub_step:");
  b.printf(" %5u", sub_.step_.count_);
  b.printf(" %10u", sub_.step_.start_time_planned_ms_ - test_start_time_ms);
  b.printf(" %10u", sub_.step_.start_time_actual_ms_ - test_start_time_ms);
  b.printf(" %10u", sub_.step_.end_time_planned_ms_ - test_start_time_ms);

  // SA (aks sub step EA)
//b.printf(" measure:");
  b.printf(" %10u", measure_.start_time_actual_ms_ - test_start_time_ms);
  // 'now' (aka measure end)
  b.printf(" %10u", now_ms - test_start_time_ms);

//b.printf("  |");
  iMeter.reportMeasurements(b, now_ms, test_start_time_ms);
//b.printf("  |");
  rpi::SensorDevices_Data_report_all(b, now_ms, test_start_time_ms);

  b.printf("  XX\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}
