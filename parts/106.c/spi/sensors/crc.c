
namespace nc {
namespace spi {
namespace sensors {

uint8_t crc(const SensorDeviceData_t * deviceData)
{
  return calcCRC8(
    (const uint8_t *) deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData->crc));
}

} // namespace sensors
} // namespace spi
} // namespace nc
