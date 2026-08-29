
namespace nc {

void Monitor::on_state_measure_report(uint32_t now_ms)
{
  (void) now_ms;

  report();

  state(State::pause_init);
}

} // namespace nc
