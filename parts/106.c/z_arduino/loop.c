
void loop()
{
  //LOG("sp=%p", cpu_sp_get());

  static uint32_t loop_count = 0;

  uint32_t now_ms = millis();
  const auto now_s = now_ms / 1000;

#if 1
  static uint32_t report_config_s = 0;
  if (now_s - report_config_s >= 5*60) {
    report_config();
    report_config_s = now_s;
    LOG("uxTaskGetStackHighWaterMark()=%u", uxTaskGetStackHighWaterMark(NULL));
  }
#endif

  static uint32_t report_count_s = 0;
  if (now_s - report_count_s >= 5) {
    Serial.print(device_tag);
    Serial.print(" now_ms="); Serial.print(now_ms);
    Serial.print(" loop_count="); Serial.println(loop_count);
    //spi_report_stats();

    report_count_s = now_s;
  }

  uartDevices_on_loop(now_ms);
  spiDevice.on_loop(now_ms);
  test.on_loop(now_ms);

  ++loop_count;
}
