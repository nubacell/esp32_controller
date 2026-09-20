
namespace nc {
namespace spi {

void Sensors::offline_config_dump() const
{
  char s[SPI_SENSORS_COUNT+1];
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    s[i] = (offline_[i]) ? '.' : 'X';
  }
  s[SPI_SENSORS_COUNT] = '\0';
  LOG("s='%s'", s);
}

} // namespace spi
} // namespace nc
