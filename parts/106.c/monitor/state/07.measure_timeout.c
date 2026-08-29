
namespace nc {

void Monitor::on_state_measure_timeout(uint32_t now_ms)
{
  (void) now_ms;

  state(State::stop);
}

} // namespace nc
