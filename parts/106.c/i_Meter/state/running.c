
/*
    After 'start_single_conversion()' 'index register' already refers to
    'Config' register and 'read_repeat()' may be used to read 'Config'
    register.
*/

void I_Meter::on_state_running(uint32_t now_ms)
{
  bool ready;
  if (!poll(ready)) {
    state(State::run_failed);
    return;
  }

  if (!ready) {
    if (timeout_time_ms_ <= now_ms) {
      LOG("WARNING: %s %s (timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64 ")",
          "i2c ADC 'conversion'", "timeout", timeout_time_ms_, now_ms,
          esp_timer_get_time());
      state(State::run_failed);
    }
    return;
  }

  uint16_t v = 0;
  const auto r = read(v);
  if (!r) {
    state(State::run_failed);
    return;
  }
  conversion_results[channel_] = v;
  //LOG("channel=%u v=0x%04X/%u/%i", channel_, v, v, int16_t(v));

  ++channel_;
  if (channel_ < I_METER_CHANNELS_COUNT) {

    timeout_time_ms_ = now_ms + single_conversion_timeout_ms;
    //LOG("timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64, timeout_time_ms_, now_ms, esp_timer_get_time());

    if (!start()) {
      state(State::run_failed);
    }
    return;
  }

  end_time_ms_ = now_ms;

  measurementsValid_ = true;

#if 0
  LOG("ms2=%" PRIu32 " - ms1=%" PRIu32 " =%" PRIu32,
      end_time_ms_, start_time_ms_, end_time_ms_ - start_time_ms_);
#endif

  state(State::idle);
}
