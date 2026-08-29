
namespace nc {

void Monitor::on_state_pause_done(uint32_t now_ms)
{
  (void) now_ms;

  state(State::measure_init);
}

} // namespace nc
