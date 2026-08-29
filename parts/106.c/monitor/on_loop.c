
namespace nc {

void Monitor::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(idle)
    CASE_CALL(start)
    CASE_CALL(stop)
    CASE_CALL(measure_init)
    CASE_CALL(measure_wait)
    CASE_CALL(measure_done)
    CASE_CALL(measure_report)
    CASE_CALL(measure_timeout)
    CASE_CALL(pause_init)
    CASE_CALL(pause_wait)
    CASE_CALL(pause_done)
#undef CASE_CALL
    default:
      LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

} // namespace nc
