
void I_Meters::on_state_adc_init(uint32_t now_ms)
{
  timeout_time_ms_ = now_ms + single_conversion_timeout_ms;

  state(
    (!adc_start())
      ? State::next_channel
      : State::adc_wait
  );
}
