
namespace nc {
namespace spi {

void Sensor::update_data()
{
  if (raw.status.transfer_done
  && raw.status.rx_bits_valid()
  && raw.status.is_crc_valid()) {
    data = raw.data;
  }
}

} // namespace spi
} // namespace nc
