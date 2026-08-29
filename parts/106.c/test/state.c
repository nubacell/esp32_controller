
void Test::state(State new_state)
{
  const auto old_state_name = to_string(state_);
  state_ = new_state;
  const auto new_state_name = to_string(state_);
  //LOG("'%s' -> '%s'", old_state_name, new_state_name);
}
