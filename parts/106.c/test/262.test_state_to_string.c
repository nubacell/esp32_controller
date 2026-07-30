
const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "step_init",
    "sub_step_init",
    "sub_step_wait",
    "sub_step_measure",
    "spi_wait",
    "sub_step_next",
    "step_next",
    "abort",
    "cleanup",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

#define TEST_STATE_STRLEN CONSTSTRLEN("/TEST/STATE sub_step_measure\n")

void test_state(const char * args)
{
  (void) args;

  char buffer[TEST_STATE_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STATE %s\n", Test::to_string(test.state_));

  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);
}
