
namespace nc {
namespace spi {
namespace sensors {

bool crc_valid(const SensorDeviceData_t * sensorDeviceData)
{
  return sensorDeviceData->crc == crc(sensorDeviceData);
}

} // namespace sensors
} // namespace spi
} // namespace nc
