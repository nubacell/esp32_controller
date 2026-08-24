
namespace nc {
namespace spi {
namespace sensors {

struct Stats
{
  unsigned rx_bits_ok;
  unsigned rx_bits_ng;
  unsigned crc_ok;
  unsigned crc_ng;
} stats_[SPI_SENSORS_COUNT];

void reportStats();
void reportStats(::nc::mk::Buffer & b);

} // namespace sensors
} // namespace spi
} // namespace nc
