
namespace nc {
namespace spi {

void Sensors::on_state_next_sensor(uint32_t now_ms)
{
  ++sensor_n_;
  if (sensor_n_ < SPI_SENSORS_COUNT) {
    state(State::transfer_init);
  } else {
    state(State::run_finished);
  }
}

} // namespace spi
} // namespace nc
