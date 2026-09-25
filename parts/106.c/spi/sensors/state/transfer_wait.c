
namespace nc {
namespace spi {

void Sensors::on_state_transfer_wait(uint32_t now_ms)
{
  if (!hw.active_) {
    auto & sensor = sensors_[sensor_n_];

    sensor.raw.status.transfer_done = true;
    ++sensor.raw.stats.transfer_done.ok();

    sensor.raw.status.rx_bits = hw.rx_bits();
    ++sensor.raw.stats.rx_bits[sensor.raw.status.rx_bits_valid()];

    sensor.raw.update_crc_valid();
    ++sensor.raw.stats.crc[sensor.raw.status.is_crc_valid()];

    sensor.update_data();

    state(State::next_sensor);
  } else
  if (timeout_time_ms_ <= now_ms) {
    auto & sensor = sensors_[sensor_n_];
    ++sensor.raw.stats.transfer_done.ng();
    state(State::transfer_timeout);
  }
}

} // namespace spi
} // namespace nc
