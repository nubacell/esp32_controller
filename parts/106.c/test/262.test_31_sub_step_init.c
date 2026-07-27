
/*
    if !step_ms skip waiting and jump directly to 'sub_step_measure'.

    '!step_ms' can be useful to do measurements at the beginning of the
    test immediately after changing PS and EL configs.

    if !sub_step_ms then next '*_sub_step_end' will be quickly executed
    just after few 'on_loop'.

    '!sub_step_ms' can be useful to do multiple repeated measurementsat
    the beginning of the test immediately after changing PS and EL configs.
*/

void Test::on_state_sub_step_init(uint32_t now_ms)
{
  // --- 'sub_step_init' ---

  const ProfileStep * profileStep = &profileSteps_[step_];
#if 1
  /*
    'sub_step_end_time_ms_' shall be relative to 'sub step [planned] start
    time'.

    Here 'sub_step_end_time_ms_' contains previous sub step 'planned' end time.
    It also acts as current sub step 'planned' start time and can differ from
    'now_ms' due to slow CPU.
  */
  const auto sub_step_start_time_ms = sub_step_end_time_ms_;
  LOG("sub_step_start_time_ms=%u now_ms=%u", sub_step_start_time_ms, now_ms);
  sub_step_end_time_ms_ = sub_step_start_time_ms + profileStep->sub_step_ms;
  LOG(". sub_step_end_time_ms_=%u", sub_step_end_time_ms_);
  state(State::sub_step_wait);
#else
  if (profileStep->step_ms) {
    sub_step_end_time_ms_ += profileStep->sub_step_ms;
    LOG(". sub_step_end_time_ms_=%u", sub_step_end_time_ms_);
    state(State::sub_step_wait);
  } else {
    state(State::sub_step_measure);
  }
#endif
}
