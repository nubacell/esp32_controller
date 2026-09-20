
namespace nc {
namespace spi {

uint8_t Sensor::Raw::crc() const
{
  return calcCRC8(
    (const uint8_t *) &data,
    LGT8F_SPI_DATA_SIZE - sizeof(data.crc));
}

} // namespace spi
} // namespace nc
