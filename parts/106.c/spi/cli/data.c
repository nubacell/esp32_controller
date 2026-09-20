
namespace nc {
namespace cli {

void spi_data(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.spi_data_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/DATA");

#if 1
  b << ::nc::spi::sensors;
#else
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    //b << ::nc::spi::sensors.sensors_[i].data;
    ::nc::spi::sensors.sensors_[i].to_buffer(b);
  }
#endif

  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace cli
} // namespace nc
