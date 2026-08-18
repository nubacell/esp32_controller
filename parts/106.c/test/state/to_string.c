
const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "step_init",
    "sub_step_init",
    "sub_step_wait",
    "measure_init",
    "measure_wait",
    "sub_step_next",
    "step_next",
    "abort",
    "cleanup",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}
