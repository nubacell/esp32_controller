
Test test;

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

#if 1
void Test::dump() const
{
  hex_dump( (const uint8_t*)this, sizeof(*this));
  LOG("step=%u sub_step=%u test_start_time_ms=%u sub_step_end_time_ms=%u"
      " measure_timeout_time_ms=%u run=%i abort=%i state=%u/%s",
      step_, sub_step_, test_start_time_ms_, sub_step_end_time_ms_,
      measure_timeout_time_ms_, run_, abort_, state_, to_string(state_));
}

void test_dump(const char * args)
{
  test.dump();
}
#endif
