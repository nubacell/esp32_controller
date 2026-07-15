
void Test::on_event(uint32_t now_ms)
{
  LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);

  const ProfileStep * profileStep = &profileSteps_[step_];
  ++sub_step_;
  if (sub_step_ < profileStep->substeps_count_) {
    event_time_ms_ += profileStep->measure_period_ms_;
  } else {
    LOG("--- step_=%u finished -- change PS and EL configs...", step_);

    ++step_;
    if (step_ < profile.steps_count_) {

      profileStep = &profileSteps_[step_];
      sub_step_ = 0;

      // Change PS and EL configs.
      change_PS_EL_configs(profileStep);

      // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
      // be executed on next 'on_loop'.
      if (profileStep->substeps_count_
      && profileStep->measure_period_ms_) {
        event_time_ms_ += profileStep->measure_period_ms_;
      }

    } else {
      deactivate_PS_EL();
      LOG("--- test finished ---");
      // 'test' finished
      state(State::idle);
    }
  }
}
