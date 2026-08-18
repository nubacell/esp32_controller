
void Test::dump() const
{
  hex_dump( (const uint8_t*)this, sizeof(*this));
  LOG("step.count_=%u"
      " sub_.step.count_=%u"
      " test_start_time_ms=%u"
      " sub_.step_.end_time_planned_ms=%u"
      " measure_.timeout_time_ms=%u"
      " run=%i"
      " abort=%i"
      " state=%u/%s",
      step_.count_,
      sub_.step_.count_,
      test_start_time_ms_,
      sub_.step_.end_time_planned_ms_,
      measure_.timeout_time_ms_,
      run_,
      abort_,
      state_, to_string(state_));
}
