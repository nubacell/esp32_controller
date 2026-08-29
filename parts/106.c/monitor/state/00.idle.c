
namespace nc {

void Monitor::on_state_idle(uint32_t now_ms)
{
  (void) now_ms;

  if (start_) {
    state(State::start);
  }
}

} // namespace nc
