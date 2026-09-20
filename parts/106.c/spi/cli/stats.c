
namespace nc {
namespace cli {

void spi_stats(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.spi_stats_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/STATS/SPI/SENSORS");

  //b << ::nc::spi::sensors.stats;
  ::nc::spi::sensors.stats_to_buffer(b);

  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace cli
} // namespace nc
