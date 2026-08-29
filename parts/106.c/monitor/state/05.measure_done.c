
namespace nc {

void Monitor::on_state_measure_done(uint32_t now_ms)
{
  (void) now_ms;

  state(State::measure_report);
}

} // namespace nc
