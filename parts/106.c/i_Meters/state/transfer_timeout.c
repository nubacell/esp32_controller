
void I_Meters::on_state_transfer_timeout(uint32_t now_ms)
{
  LOG("WARNING: %s %s (timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64 ")",
      "i2c ADC 'conversion'", "timeout", timeout_time_ms_, now_ms,
      esp_timer_get_time());

  state(State::next_channel);
}
