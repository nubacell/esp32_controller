
namespace nc {
namespace spi {

bool Sensor::Raw::is_crc_valid_impl() const
{
  return data.crc == crc();
}

} // namespace spi
} // namespace nc
