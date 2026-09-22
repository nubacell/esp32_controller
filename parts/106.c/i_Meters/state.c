
void I_Meters::state(State new_state)
{
#if defined iMeters_state_verbose
  const auto old_state_name = to_string(state_);
#endif
  state_ = new_state;
#if defined iMeters_state_verbose
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
#endif
}
