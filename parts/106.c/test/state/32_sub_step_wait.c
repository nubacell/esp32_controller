
// Long running state. Check for user's abort request.

// 'sub_step_wait'
void Test::on_state_sub_step_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (sub_.step_.end_time_planned_ms_ <= now_ms) {
#if 0
    // --- 'sub_step_end' ---
    LOG("--- sub_.step_.count_=%u finished -- do measurements...", sub_.step_.count_);
#endif

    state(State::measure_init);
  }
}
