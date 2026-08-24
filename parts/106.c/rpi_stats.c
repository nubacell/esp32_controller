
namespace rpi {

#define STATS_STRLEN ( \
  CONSTSTRLEN("/STATS") \
  \
  + CONSTSTRLEN(" i2c/adc") \
  + ( \
    + CONSTSTRLEN("  0:") \
    + CONSTSTRLEN(" start=1234567890/1234567890") \
    + CONSTSTRLEN(" poll=1234567890/1234567890") \
    + CONSTSTRLEN(" read=1234567890/1234567890") \
  ) * I_METER_CHANNELS_COUNT \
  \
  + CONSTSTRLEN(" spi") \
  + CONSTSTRLEN(" rx_bits=ok/ng") \
  + CONSTSTRLEN(" 999/999") * SPI_SENSORS_COUNT \
  + CONSTSTRLEN(" crc=on/ng") \
  + CONSTSTRLEN(" 999/999") * SPI_SENSORS_COUNT \
  \
  + CONSTSTRLEN("\n") \
)

void reportStats()
{
  char buffer[STATS_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/STATS");

  b.printf(" i2c/adc");
  iMeter.reportStats(b);

  b.printf(" spi");
  ::nc::spi::sensors::reportStats(b);

  b.printf("\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}

} // namespace rpi
