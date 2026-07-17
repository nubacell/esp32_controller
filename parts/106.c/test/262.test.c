
Test test;

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

void Test::dump() const
{
  hex_dump( (const uint8_t*)this, sizeof(*this));
  LOG("step=%u sub_step=%u start_time_ms=%u sub_step_end_time_ms=%u"
      " spi_timeout_time_ms=%u run=%i abort=%i state=%u/%s",
      step_, sub_step_, start_time_ms_, sub_step_end_time_ms_,
      spi_timeout_time_ms_, run_, abort_, state_, to_string(state_));
}

void test_dump(const char * args)
{
  test.dump();
}
