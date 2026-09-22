
void I_Meters::on_state_run_finished(uint32_t now_ms)
{
  end_time_ms_ = now_ms;
  run_finished_ = true;
  state(State::idle);
}
