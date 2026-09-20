
::nc::mk::Buffer &
::nc::spi::Sensor::Raw::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << status
    << data;
}
