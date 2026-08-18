
void loop()
{
  //LOG("sp=%p", cpu_sp_get());

  static uint32_t loop_count = 0;

  uint32_t now_ms = millis();
  const auto now_s = now_ms / 1000;

#if 1
  // FIXME: Refactor using 'Runnable'
  static uint32_t report_config_s = 0;
  static constexpr uint32_t report_config_period_s = 5*60;
  if (now_s - report_config_s >= report_config_period_s) {
    report_config_s = now_s;
    report_config();
  }
#endif

  // FIXME: Refactor using 'Runnable'
  static uint32_t report_stats_s = 0;
  static constexpr uint32_t report_stats_period_s = 5;
  if (now_s - report_stats_s >= report_stats_period_s) {
    report_stats_s = now_s;
    // FIXME: Move to 'report_stats()'
    Serial.print(device_tag);
    Serial.print(" now_ms="); Serial.print(now_ms);
    Serial.print(" loop_count="); Serial.println(loop_count);
    //report_stats();
  }

  uartDevices_on_loop(now_ms);
  iMeter.on_loop(now_ms);
  spiDevice.on_loop(now_ms);
  test.on_loop(now_ms);

  ++loop_count;
}
