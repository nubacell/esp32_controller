
// Can be used to implement delay before init retry
void I_Meters::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
  state(State::idle);
}
