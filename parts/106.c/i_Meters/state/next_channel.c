
void I_Meters::on_state_next_channel(uint32_t now_ms)
{
  ++channel_;
  if (channel_ < I_METERS_COUNT) {
    state(State::transfer_init);
  } else {
    state(State::run_finished);
  }
}
