
// Long running state. Check for user's abort request.

// 'sub_step_wait'
void Test::on_state_sub_step_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (sub_step_end_time_ms_ <= now_ms) {
    // --- 'sub_step_end' ---
    LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);
    state(State::sub_step_measure);
  }
}
