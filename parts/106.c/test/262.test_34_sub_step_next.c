
void Test::on_state_sub_step_next(uint32_t now_ms)
{
  // 'sub_step_next'

  const ProfileStep * profileStep = &profileSteps_[step_];
  ++sub_step_;
  LOG(". sub_step_=%u", sub_step_);
  if (sub_step_ < profileStep->substeps_count_) {
    sub_step_end_time_ms_ += profileStep->measure_period_ms_;
    state(State::sub_step_init);
  } else {
    LOG("--- step_=%u finished -- start next step (change PS and EL configs) ...", step_);
    state(State::step_next);
  }
}
