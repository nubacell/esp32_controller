
namespace nc {
namespace spi {
namespace sensors {

inline SensorDeviceData_t * deviceData(unsigned sensor_n)
{
  return (SensorDeviceData_t*) (&::nc::spi::buffers::rx[sensor_n][0]);
}

} // namespace sensors
} // namespace spi
} // namespace nc
