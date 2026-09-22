
// 'measure_init'
void Test::on_state_measure_init(uint32_t now_ms)
{
  // --- 'measure_init' ---
  measure_.start_time_actual_ms_ = now_ms;

  measure_.timeout_time_ms_ = now_ms + measure_.timeout_ms;

  ::nc::spi::sensors.run();
  iMeters.run();

  state(State::measure_wait);
}
