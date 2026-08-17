
void Test::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(idle)
    CASE_CALL(step_init)
    CASE_CALL(sub_step_init)
    CASE_CALL(sub_step_wait)
    CASE_CALL(sub_step_measure)
    CASE_CALL(measure_wait)
    CASE_CALL(sub_step_next)
    CASE_CALL(step_next)
    CASE_CALL(abort)
    CASE_CALL(cleanup)
#undef CASE_CALL
    default:
      LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}
