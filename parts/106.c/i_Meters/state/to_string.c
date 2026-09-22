
const char * I_Meters::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "transfer_init",
    "transfer_wait",
    "transfer_timeout",
    "next_sensor",
    "run_finished",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}
