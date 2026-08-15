
void I_Meter::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    measurementsValid_ = false;

    start_time_us_ = esp_timer_get_time();
    start_time_ms_ = now_ms;

    channel_ = 0;

    //constexpr uint32_t single_conversion_timeout_ms = 2;
    timeout_time_ms_ = now_ms + single_conversion_timeout_ms;
    //LOG("timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64, timeout_time_ms_, now_ms, esp_timer_get_time());

    state(
      (start())
        ? State::running
        : State::run_failed
    );
  }
}
