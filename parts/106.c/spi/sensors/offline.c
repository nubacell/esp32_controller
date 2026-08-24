
namespace nc {
namespace spi {
namespace sensors {

bool offline_[SPI_SENSORS_COUNT];

uint8_t skip_offline(uint8_t spi_sensor_n)
{
  //LOG("spi_sensor_n=%u", spi_sensor_n);
  while (spi_sensor_n < SPI_SENSORS_COUNT
      && offline_[spi_sensor_n]) {
    ++spi_sensor_n;
    //LOG("spi_sensor_n=%u", spi_sensor_n);
  }
  return spi_sensor_n;
}

void config(uint32_t v)
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    offline_[i] = true;
  }
  //LOG("v=0x%X/%u", v, v);
  while (v) {
    unsigned i = v % 10;
    //LOG("v=0x%X/%u", i, i);
    if (i) {
      --i;
      if (i < SPI_SENSORS_COUNT) {
        offline_[i] = false;
      }
    }
    v /= 10;
  }
}

void config_dump()
{
  char s[SPI_SENSORS_COUNT+1];
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    s[i] = (offline_[i]) ? '.' : 'X';
  }
  s[SPI_SENSORS_COUNT] = '\0';
  LOG("s='%s'", s);
}

} // namespace sensors
} // namespace spi
} // namespace nc
