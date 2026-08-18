
void SPI_Device::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(init_failed)
    CASE_CALL(idle)
    CASE_CALL(running)
    CASE_CALL(run_failed)
#undef CASE_CALL
    default:
      //LOG("ERROR: Unexpected state=%u", state_);
      LOG("ERROR: Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}
