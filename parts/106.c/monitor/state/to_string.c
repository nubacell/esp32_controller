
namespace nc {

const char * Monitor::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "start",
    "stop",
    "measure_init",
    "measure_wait",
    "measure_done",
    "measure_report",
    "measure_timeout",
    "pause_init",
    "pause_wait",
    "pause_done",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

} // namespace nc
