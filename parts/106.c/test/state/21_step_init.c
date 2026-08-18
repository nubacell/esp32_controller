
#define TEST_STEP_START_STRLEN CONSTSTRLEN("/TEST/STEP 99\n")

void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---
  step_.start_time_actual_ms_ = now_ms;

  char buffer[TEST_STEP_START_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STEP %2u\n", step_.count_);
  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);

  const ProfileStep * profileStep = &profileSteps_[step_.count_];

  // To reduce 'time drift' 'step planned end time' shall be relative to 'step
  // planned start time' instead of 'step actual start time'.
  // FIXME: Don't forget to update 'step_.start_time_planned_ms_' for 'next step'
  step_.end_time_planned_ms_ = step_.start_time_planned_ms_ + profileStep->step_ms;

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_.step_.count_ = 0;
  sub_.step_.start_time_planned_ms_ = step_.start_time_planned_ms_;

  state(State::sub_step_init);
}
