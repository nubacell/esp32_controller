
namespace nc {

void Monitor::on_state_measure_wait(uint32_t now_ms)
{
  if (stop_) {
    state(State::stop);
  } else
  if (::nc::spi::sensors.run_finished()
  && iMeter.isMeasurementFinished()) {
    state(State::measure_done);
  } else
  if (measure_.timeout_time_ms_ <= now_ms) {
    LOG("ERROR: measurements timeout");
    state(State::measure_timeout);
  }
}

} // namespace nc
