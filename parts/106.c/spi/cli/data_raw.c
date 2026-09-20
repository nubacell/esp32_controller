
namespace nc {
namespace cli {

void spi_data_raw(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.spi_raw_data_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/RAW/DATA");

  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    //b << ::nc::spi::sensors.sensors_[i].raw.data;
    b << ::nc::spi::sensors.sensors_[i].raw;
  }

  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace cli
} // namespace nc
