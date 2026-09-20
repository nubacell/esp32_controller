
::nc::mk::Buffer &
::nc::spi::Sensors::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b << ::nc::spi::sensors.sensors_[i];
  }
  return b;
}
