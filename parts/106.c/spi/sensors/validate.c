
namespace nc {
namespace spi {
namespace sensors {

bool crc_valid_[SPI_SENSORS_COUNT];
bool valid_[SPI_SENSORS_COUNT];
bool valid_all_;

void validate()
{
  valid_all_ = true;
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    if (!offline_[i]) {
      const bool rx_bits_valid = !(spi_rx_bits[i] < SPI_RX_BITS_MIN);
      if (rx_bits_valid) {
        ++::nc::spi::sensors::stats_[i].rx_bits_ok;
      } else {
        ++::nc::spi::sensors::stats_[i].rx_bits_ng;
      }
      const auto crc_valid = ::nc::spi::sensors::crc_valid(deviceData(i));
      crc_valid_[i] = crc_valid;
      if (crc_valid) {
        ++::nc::spi::sensors::stats_[i].crc_ok;
      } else {
        ++::nc::spi::sensors::stats_[i].crc_ng;
      }
      const bool sensor_valid = rx_bits_valid & crc_valid;
      valid_[i] = sensor_valid;
      valid_all_ &= sensor_valid;
    }
  }
}

#define crc_valid_STRLEN ( \
  CONSTSTRLEN(" OK") * SPI_SENSORS_COUNT \
)

void crc_valid_dump()
{
  char buffer[crc_valid_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b.printf(" %s", crc_valid_[i] ? "OK" : "NG");
  }
  LOG("crc_valid='%s'", buffer);
}

#define valid_STRLEN ( \
  CONSTSTRLEN(" OK") * SPI_SENSORS_COUNT \
)

void valid_dump()
{
  char buffer[valid_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b.printf(" %s", valid_[i] ? "OK" : "NG");
  }
  LOG("valid='%s'", buffer);
}

} // namespace sensors
} // namespace spi
} // namespace nc
