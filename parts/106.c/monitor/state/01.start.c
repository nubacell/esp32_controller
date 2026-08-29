
namespace nc {

void Monitor::on_state_start(uint32_t now_ms)
{
  (void) now_ms;

  start_ = false;

  active_ = true;

  pause_.end_time_ms_ = now_ms;

  state(State::measure_init);
}

} // namespace nc
