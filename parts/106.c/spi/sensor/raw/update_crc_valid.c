
namespace nc {
namespace spi {

void Sensor::Raw::update_crc_valid()
{
  status.crc_valid_ = is_crc_valid_impl();
}

} // namespace spi
} // namespace nc
