
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

      // timings for all measurement sub steps will be based on 'test_start'
      // time
      start_time_ms_ = sub_step_end_time_ms_ = now_ms;

      // --- 'step_0' ---
      step_ = 0;

      state(State::step_init);
    }
  }
}
