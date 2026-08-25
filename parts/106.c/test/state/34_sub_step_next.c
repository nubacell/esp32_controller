
void Test::on_state_sub_step_next(uint32_t now_ms)
{
  // --- 'sub_step_next' ---

  //const ProfileStep * profileStep = &profileSteps_[step_.count_];
  ++sub_.step_.count_;
  //LOG(". sub_.step_.count_=%u", sub_.step_.count_);
#if 1
  if (sub_.step_.count_ > sub_step_count_max_) {
    LOG("ERROR: sub_.step_.count_=%u limit=%u exceeded",
        sub_.step_.count_, sub_step_count_max_);
    state(State::abort);
  }
#endif
  if (now_ms < step_.end_time_planned_ms_) {
    //LOG(". now_ms=%u < step_.end_time_planned_ms_=%u", now_ms, step_.end_time_planned_ms_);
    sub_.step_.start_time_planned_ms_ = sub_.step_.end_time_planned_ms_;
    state(State::sub_step_init);
  } else {
    //LOG("--- step_=%u finished -- start next step (change PS and EL configs) ...", step_.count_);
    state(State::step_next);
  }
}
