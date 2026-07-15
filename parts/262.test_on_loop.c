
void Test::on_loop(uint32_t now_ms) override
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(idle)
    CASE_CALL(running)
#undef CASE_CALL
    default:
      LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}
