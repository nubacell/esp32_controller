
//namespace nc {
//namespace spi {

::nc::mk::Buffer &
::nc::spi::Sensor::Raw::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << transfer_init
    << transfer_done
    << rx_bits
    << crc;
}

//} // namespace spi
//} // namespace nc
