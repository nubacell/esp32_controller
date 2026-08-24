
namespace nc {
namespace spi {
namespace sensors {

#define SPI_STATS_STRLEN ( \
  CONSTSTRLEN(" rx_bits=ok/ng") \
  + CONSTSTRLEN(" 999/999") * SPI_SENSORS_COUNT \
  + CONSTSTRLEN(" crc=on/ng") \
  + CONSTSTRLEN(" 999/999") * SPI_SENSORS_COUNT \
)

void reportStats(::nc::mk::Buffer & b)
{
  b.printf(" rx_bits=ok/ng");
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b.printf(" %u/%u", stats_[i].rx_bits_ok, stats_[i].rx_bits_ng);
  }
  b.printf(" crc=ok/ng");
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b.printf(" %u/%u", stats_[i].crc_ok, stats_[i].crc_ng);
  }
}

void reportStats()
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    LOG(" [%u] rx_bits=%u/%u crc=%u/%u",
        i,
        stats_[i].rx_bits_ok, stats_[i].rx_bits_ng,
        stats_[i].crc_ok, stats_[i].crc_ng);
  }
}

} // namespace sensors
} // namespace spi
} // namespace nc
