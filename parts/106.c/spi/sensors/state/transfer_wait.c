
namespace nc {
namespace spi {

void Sensors::on_state_transfer_wait(uint32_t now_ms)
{
  if (!hw.active_) {
    auto & sensor = sensors_[sensor_n_];

    sensor.raw.status.transfer_done = true;
    sensor.raw.status.rx_bits = hw.rx_bits();
    sensor.raw.update_crc_valid();
    sensor.update_data();

    ++sensor.stats.transfer.ok();
    ++sensor.stats.rx_bits[sensor.raw.status.rx_bits_valid()];
    ++sensor.stats.crc[sensor.raw.status.is_crc_valid()];

    state(State::next_sensor);
  } else
  if (timeout_time_ms_ <= now_ms) {
    ++sensors_[sensor_n_].stats.transfer.ng();
    state(State::transfer_timeout);
  }
}

} // namespace spi
} // namespace nc
