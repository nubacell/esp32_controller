
namespace nc {

void Monitor::on_state_pause_wait(uint32_t now_ms)
{
  if (stop_) {
    state(State::stop);
  } else
  if (pause_.end_time_ms_ <= now_ms) {
    state(State::pause_done);
  }
}

} // namespace nc
