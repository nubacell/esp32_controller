
void Test::on_state_step_next(uint32_t now_ms)
{
  // --- 'step_next' ---
  ++step_;
  LOG(". step_=%u", step_);
  if (step_ < profile.steps_count_) {
    state(State::step_init);
  } else {
    state(State::cleanup);
  }
}
