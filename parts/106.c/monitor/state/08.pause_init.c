
namespace nc {

void Monitor::on_state_pause_init(uint32_t now_ms)
{
  pause_.end_time_ms_ += pause_.period_ms_;

  state(State::pause_wait);
}

} // namespace nc
