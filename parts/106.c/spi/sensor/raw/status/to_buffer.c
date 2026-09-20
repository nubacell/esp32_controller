
::nc::mk::Buffer &
::nc::spi::Sensor::Raw::Status::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  union {
    struct {
      uint8_t transfer:1;
      uint8_t bits:1;
      uint8_t crc:1;
    } as_bits;
    uint8_t as_byte;
  } errors = {
    .as_bits = {
      .transfer = !transfer_done,
      .bits = !rx_bits_valid(),
      .crc = !is_crc_valid(),
    },
  };
  return b
    << ' '
    << errors.as_byte;
}
