
const char * SPI_Device::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "running",
    "run_failed",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}
