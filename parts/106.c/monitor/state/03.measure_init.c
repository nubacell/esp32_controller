
namespace nc {

void Monitor::on_state_measure_init(uint32_t now_ms)
{
  measure_.timeout_time_ms_ = now_ms + measure_.timeout_ms;

  spiDevice.run();
  iMeter.run();

  state(State::measure_wait);
}

} // namespace nc
