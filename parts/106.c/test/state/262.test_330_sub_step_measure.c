
// 'sub_step_measure'
void Test::on_state_sub_step_measure(uint32_t now_ms)
{
  // --- 'sub_step_measure' ---

  // --- 'measure_init' ---
  // FIXME: Use correct 'measure_timeout_ms'
  constexpr uint32_t measure_timeout_ms = 10;
  measure_timeout_time_ms_ = now_ms + measure_timeout_ms;

#if 0
  LOG(". measure_timeout_time_ms_=%u", measure_timeout_time_ms_);
#endif
  spiDevice.run();
  iMeter.run();

  state(State::measure_wait);
}
