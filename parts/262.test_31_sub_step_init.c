
/*
    if !substeps_count_ skip waiting and jump directly to 'sub_step_measure'.

    '!substeps_count_' can be useful to do measurements at the beginning of the
    test immediately after changing PS and EL configs.

    if !measure_period_ms_ then next '*_sub_step_end' will be quickly executed
    just after few 'on_loop'.

    '!measure_period_ms_' can be useful to do multiple repeated measurementsat
    the beginning of the test immediately after changing PS and EL configs.
*/

void Test::on_state_sub_step_init(uint32_t now_ms)
{
  // --- 'sub_step_init' ---

  const ProfileStep * profileStep = &profileSteps_[step_];
  if (profileStep->substeps_count_) {
    sub_step_end_time_ms_ += profileStep->measure_period_ms_;
    LOG(". sub_step_end_time_ms_=%u", sub_step_end_time_ms_);
    state(State::sub_step_wait);
  } else {
    state(State::sub_step_measure);
  }
}
