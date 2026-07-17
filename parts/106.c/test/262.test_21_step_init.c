
void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---

  const ProfileStep * profileStep = &profileSteps_[step_];

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_step_ = 0;

  state(State::sub_step_init);
}
