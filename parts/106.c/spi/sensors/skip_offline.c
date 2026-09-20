
namespace nc {
namespace spi {

void Sensors::skip_offline()
{
  //LOG("sensor_n_=%u", sensor_n_);
  while (sensor_n_ < SPI_SENSORS_COUNT
      && offline_[sensor_n_]) {
    ++sensor_n_;
    //LOG("sensor_n_=%u", sensor_n_);
  }
}

} // namespace spi
} // namespace nc
