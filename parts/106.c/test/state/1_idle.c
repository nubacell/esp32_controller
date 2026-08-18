
void Test::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    abort_ = false;

    if (!profile.steps_count_) {
      LOG("ERROR: profile is empty");
      state(State::idle);
    } else {

      // --- 'test_init' ---

      // timings for all steps and all measurement sub steps will be based on
      // 'test_start' time
      test_start_time_ms_               = now_ms;
      step_.start_time_planned_ms_      = now_ms;
      step_.end_time_planned_ms_        = now_ms; // FIXME: This line is for safety.
      sub_.step_.start_time_planned_ms_ = now_ms;
      sub_.step_.end_time_planned_ms_   = now_ms; // FIXME: This line is for safety.
#if 0
      LOG(". TEST test_start_time_ms_=%u", test_start_time_ms_);
      LOG(". TEST step_.start_time_planned_ms_=%u", step_.start_time_planned_ms_);
      LOG(". TEST step_.end_time_planned_ms_=%u", step_.end_time_planned_ms_);
      LOG(". TEST sub_.step_.start_time_planned_ms_=%u", sub_.step_.start_time_planned_ms_);
      LOG(". TEST sub_.step_.end_time_planned_ms_=%u", sub_.step_.end_time_planned_ms_);
#endif

      // --- 'step_0' ---
      step_.count_ = 0;

      state(State::step_init);
    }
  }
}
