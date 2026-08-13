
void I_Meter::on_state_run_failed(uint32_t now_ms)
{
  (void) now_ms;
  state(State::idle);
}
