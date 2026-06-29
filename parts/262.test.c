
Test test;

const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "idle",
    "running",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void test_run(const char * args)
{
  (void) args;
  test.run();
}

void test_abort(const char * args)
{
  (void) args;
  test.abort();
}
