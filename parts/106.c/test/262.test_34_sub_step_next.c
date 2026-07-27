
void Test::on_state_sub_step_next(uint32_t now_ms)
{
  // 'sub_step_next'

  const ProfileStep * profileStep = &profileSteps_[step_];
  ++sub_step_;
  LOG(". sub_step_=%u", sub_step_);
  if (now_ms < step_end_time_ms_) {
//  sub_step_end_time_ms_ += profileStep->sub_step_ms;
    state(State::sub_step_init);
  } else {
    LOG("--- step_=%u finished -- start next step (change PS and EL configs) ...", step_);
    state(State::step_next);
  }
}
