
// Long running state. Check for user's abort request.

// 'sub_step_wait'
void Test::on_state_sub_step_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (sub_step_end_time_ms_ <= now_ms) {

    // --- 'sub_step_end' ---
    LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);

    // --- 'step_measure' ---

    // --- 'spi_init' ---
    // FIXME: Use correct 'spi_timeout_ms'
    constexpr uint32_t spi_timeout_ms = 50;
    //constexpr uint32_t spi_timeout_ms = 5;
    spi_timeout_time_ms_ = now_ms + spi_timeout_ms;
    LOG(". spi_timeout_time_ms_=%u", spi_timeout_time_ms_);
    spiDevice.run();
    state(State::spi_wait);

  }
}
