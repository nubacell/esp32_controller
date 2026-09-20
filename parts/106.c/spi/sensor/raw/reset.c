
namespace nc {
namespace spi {

void Sensor::Raw::reset()
{
  memset(tx_buffer, 0, sizeof(tx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  status.transfer_done = false;
  status.rx_bits = 0;
  status.crc_valid_ = false;
}

} // namespace spi
} // namespace nc
