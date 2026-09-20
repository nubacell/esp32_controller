
namespace nc {
namespace spi {

void Sensors::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    run_finished_ = false;

    transfer_start_time_ms_ = now_ms;
    sensor_n_ = 0;
    state(State::transfer_init);
  }
}

} // namespace spi
} // namespace nc
