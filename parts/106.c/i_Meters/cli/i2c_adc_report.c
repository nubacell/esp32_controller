
#define I_Meter_Measurements_STRLEN ( \
  CONSTSTRLEN("/I2C/ADC/iMeter/DATA") \
  \
  + ( \
    + CONSTSTRLEN(" 0") \
    + CONSTSTRLEN(" 61440000") \
  ) * I_METERS_COUNT \
  \
  + CONSTSTRLEN(" XX") \
  + CONSTSTRLEN("\n") \
)

void i2c_adc_report(const char * args)
{
  (void) args;

  char buffer[I_Meter_Measurements_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/I2C/ADC/iMeter/DATA");
  b << iMeters;
  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}
