
void Test::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    abort_ = false;

    if (!profile.steps_count_) {
      LOG("ERROR: profile is empty");
      state(State::idle);
    } else {
      state(State::running);

      step_ = 0;
      sub_step_ = 0;

      const ProfileStep * profileStep = &profileSteps_[step_];

      //start_time_ms_ = now_ms;
      event_time_ms_ = now_ms;

      // Change PS and EL configs.
      change_PS_EL_configs(profileStep);

      // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
      // be executed on next 'on_loop'.
      if (profileStep->substeps_count_
      && profileStep->measure_period_ms_) {
        event_time_ms_ += profileStep->measure_period_ms_;
      }
    }
  }
}
