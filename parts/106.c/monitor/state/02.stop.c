
namespace nc {

void Monitor::on_state_stop(uint32_t now_ms)
{
  (void) now_ms;

  stop_ = false;
  active_ = false;
  state(State::idle);
}

} // namespace nc
