
//namespace nc {
//namespace spi {

::nc::mk::Buffer &
::nc::spi::Sensor::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
/*
  LOG("&transfer=%p counter: [0]=%" PRIu64 " [1]=%" PRIu64 "",
      &transfer,
      transfer.counter_[0],
      transfer.counter_[1]);
  hex_dump( (const uint8_t *) &transfer, sizeof(transfer));
*/

  return b
    << transfer
    << rx_bits
    << crc;
}

//} // namespace spi
//} // namespace nc
