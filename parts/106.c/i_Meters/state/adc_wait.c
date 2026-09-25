
/*
    After 'start_single_conversion()' 'index register' already refers to
    'Config' register and 'read_repeat()' may be used to read 'Config'
    register.
*/

void I_Meters::on_state_adc_wait(uint32_t now_ms)
{
  bool ready;
  if (!adc_poll(ready)) {
    state(State::next_channel);
    return;
  }

  if (!ready) {
    if (timeout_time_ms_ <= now_ms) {
      state(State::adc_timeout);
    }
    return;
  }

  uint16_t v = 0;
  const auto r = adc_read(v);
  if (r) {
    iMeters_[channel_].value = v;
    iMeters_[channel_].status.adc_done = true;;
  }

  state(State::next_channel);
}
