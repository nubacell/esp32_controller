
void I_Meters::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    run_finished_ = false;

    start_time_ms_ = now_ms;
    channel_ = 0;

    state(State::transfer_init);
  }
}
