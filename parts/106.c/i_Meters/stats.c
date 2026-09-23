
namespace nc {
namespace i2c {
namespace stats {

#define I_Meter_Stats_STRLEN ( \
  CONSTSTRLEN("/STATS/I2C/ADC/iMeter") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" XX") \
  + CONSTSTRLEN("\n") \
)

void report()
{
  char buffer[I_Meter_Stats_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/STATS/I2C/ADC/iMeter");

  b << iMeters.stats;

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace stats
} // namespace i2c
} // namespace nc
