
::nc::mk::Buffer &
::nc::spi::Sensor::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << raw.status
    << data;
}
