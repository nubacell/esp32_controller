
void Test::on_state_step_next(uint32_t now_ms)
{
  // --- 'step_next' ---
  ++step_.count_;
  //LOG(". step_.count_=%u", step_.count_);
  if (step_.count_ < profile.steps_count_) {
    step_.start_time_planned_ms_ = step_.end_time_planned_ms_;
    state(State::step_init);
  } else {
    state(State::cleanup);
  }
}
