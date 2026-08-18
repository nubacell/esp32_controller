
/*
    if !step_ms skip waiting and jump directly to 'measure'.

    '!step_ms' can be useful to do measurements at the beginning of the
    test immediately after changing PS and EL configs.

    if !sub_step_ms then next '*_sub_step_end' will be quickly executed
    just after few 'on_loop'.

    '!sub_step_ms' combined with non-zero 'step_ms' can be useful to do
    multiple repeated measurements at the beginning of the test immediately
    after changing PS and EL configs.
*/

void Test::on_state_sub_step_init(uint32_t now_ms)
{
  // --- 'sub_step_init' ---
  sub_.step_.start_time_actual_ms_ = now_ms;

  const ProfileStep * profileStep = &profileSteps_[step_.count_];

  // To reduce 'time drift' 'sub step planned end time' shall be relative to
  // 'sub step planned start time' instead of 'sub step actual start time'.
  // FIXME: Don't forget to update 'sub_.step_.start_time_planned_ms for 'next sub step'
  sub_.step_.end_time_planned_ms_ = sub_.step_.start_time_planned_ms_ + profileStep->sub_step_ms;

  state(State::sub_step_wait);
}
