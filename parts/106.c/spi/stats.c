
namespace nc {
namespace spi {
namespace stats {

void report()
{
  auto & buffer = ::nc::shared_buffers.spi_stats_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/SENSORS/STATS");

  ::nc::spi::sensors.stats_to_buffer(b);

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace stats
} // namespace spi
} // namespace nc
