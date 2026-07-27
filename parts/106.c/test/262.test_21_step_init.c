
#define TEST_STEP_START_STRLEN CONSTSTRLEN("/TEST/STEP 99\n")

void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---

  char buffer[TEST_STEP_START_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STEP %2u\n", step_);
  if (used < buffer_size) {
    Serial.write(buffer, used);
    //LOG("buffer_size=%zu required=%zu", buffer_size, used + 1);
  } else {
    buffer[buffer_size - 2] = '\n';
    buffer[buffer_size - 1] = '\0';
    Serial.write(buffer, buffer_size - 1);
    LOG("ERROR: buffer overflow (buffer_size=%zu required=%zu)", buffer_size, used + 1);
  }

  const ProfileStep * profileStep = &profileSteps_[step_];

  /*
    'step_end_time_ms_' shall be relative to 'step [planned] start time'.

    Here 'step_end_time_ms_' contains previous step 'planned' end time. It also
    acts as current step 'planned' start time and can differ from 'now_ms' due
    to slow CPU.
  */
  const auto step_start_time_ms = step_end_time_ms_;
#if 0
  LOG("DEBUG: step_start_time_ms=%u '%s' now_ms=%u",
      step_start_time_ms,
      (step_start_time_ms == now_ms) ? "==" : "!=",
      now_ms);
#endif
  sub_step_end_time_ms_ = step_start_time_ms;

  step_end_time_ms_ += profileStep->step_ms;
#if 0
  LOG(". step_end_time_ms_=%u", step_end_time_ms_);
#endif

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_step_ = 0;

  state(State::sub_step_init);
}
