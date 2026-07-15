
void Test::on_state_running(uint32_t now_ms)
{
  if (abort_) {
    abort_ = false;

    deactivate_PS_EL();

    state(State::idle);
  } else
  if (event_time_ms_ <= now_ms) {
    on_event(now_ms);
  }
}
