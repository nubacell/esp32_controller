
void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---

  const ProfileStep * profileStep = &profileSteps_[step_];

  /*
    'step_end_time_ms_' shall be relative to 'step [planned] start time'.

    Here 'step_end_time_ms_' contains previous step 'planned' end time. It also
    acts as current step 'planned' start time and can differ from 'now_ms' due
    to slow CPU.
  */
  const auto step_start_time_ms = step_end_time_ms_;
  LOG("DEBUG: step_start_time_ms=%u '%s' now_ms=%u",
      step_start_time_ms,
      (step_start_time_ms == now_ms) ? "==" : "!=",
      now_ms);
  sub_step_end_time_ms_ = step_start_time_ms;

  step_end_time_ms_ += profileStep->step_ms;
  LOG(". step_end_time_ms_=%u", step_end_time_ms_);

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_step_ = 0;

  state(State::sub_step_init);
}
