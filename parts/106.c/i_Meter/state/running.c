
/*
    After 'start_single_conversion()' 'index register' already refers to
    'Config' register and 'read_repeat()' may be used to read 'Config'
    register.
*/

void I_Meter::on_state_running(uint32_t now_ms)
{
  ::nc::i2c::adc_regs::Config c = {};
  if (!::nc::i2c::adc.read_repeat( *(uint16_t*) &c)) {
    state(State::run_failed);
    return;
  }

  //LOG("sizeof(::nc::i2c::adc_regs::Config)=%zu", sizeof(::nc::i2c::adc_regs::Config));
  //::nc::i2c::adc_regs::dump(c);

  const auto converting = !c.OS; // 0 == ADC performs conversion
  if (converting) {
    if (timeout_time_ms_ <= now_ms) {
      LOG("WARNING: %s %s (timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64 ")",
          "i2c ADC 'conversion'", "timeout", timeout_time_ms_, now_ms,
          esp_timer_get_time());
      state(State::run_failed);
    }
    return;
  }

  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
  if (!r) {
    state(State::run_failed);
    return;
  }
  conversion_results[channel_] = v;
  //LOG("channel=%u v=0x%04X/%u/%i", channel_, v, v, int16_t(v));

  ++channel_;
  if (channel_ < I_METER_CHANNELS_COUNT) {

    //constexpr uint32_t single_conversion_timeout_ms = 2;
    timeout_time_ms_ = now_ms + single_conversion_timeout_ms;
    //LOG("timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64, timeout_time_ms_, now_ms, esp_timer_get_time());

    if (!::nc::i2c::adc.start_single_conversion(channel_)) {
      state(State::run_failed);
    }
    return;
  }

  finish_time_us_ = esp_timer_get_time();
  finish_time_ms_ = now_ms;

  measurementsValid_ = true;

  LOG("us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64,
      finish_time_us_, start_time_us_, finish_time_us_ - start_time_us_);
  LOG("ms2=%" PRIu32 " - ms1=%" PRIu32 " =%" PRIu32,
      finish_time_ms_, start_time_ms_, finish_time_ms_ - start_time_ms_);

  for (unsigned i = 0; i < I_METER_CHANNELS_COUNT; ++i) {
    const auto v = conversion_results[i];
    const auto volts = double(v) * 6.144 / 32768.;
    LOG("channel=%u v=0x%04X/%u/%i volts=%f", i, v, v, int16_t(v), volts);
  }

  iMeter.reportMeasurements(0, 0);

  state(State::idle);
}
