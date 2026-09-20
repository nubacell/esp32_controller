
namespace nc {
namespace spi {

void Sensors::on_state_transfer_init(uint32_t now_ms)
{
  sensors_[sensor_n_].raw.reset();

  if (offline_[sensor_n_]) {
    state(State::next_sensor);
    return;
  }

  timeout_time_ms_ = now_ms + single_transfer_timeout_ms;
  if (!hw_transfer_start()) {
    ++sensors_[sensor_n_].stats.transfer.ng();
    state(State::next_sensor);
  } else {
    state(State::transfer_wait);
  }
}

} // namespace spi
} // namespace nc
