
// 'sub_step_measure'
void Test::on_state_sub_step_measure(uint32_t now_ms)
{
  // --- 'sub_step_measure' ---

  // --- 'spi_init' ---
  // FIXME: Use correct 'spi_timeout_ms'
  constexpr uint32_t spi_timeout_ms = 50;
  //constexpr uint32_t spi_timeout_ms = 5;
  spi_timeout_time_ms_ = now_ms + spi_timeout_ms;
  LOG(". spi_timeout_time_ms_=%u", spi_timeout_time_ms_);
  spiDevice.run();
  state(State::spi_wait);
}
