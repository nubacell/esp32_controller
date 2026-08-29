
void loop()
{
  uint32_t now_ms = millis();

  ::nc::loop::stats_.on_loop(now_ms);

  uartDevices_on_loop(now_ms);
  iMeter.on_loop(now_ms);
  spiDevice.on_loop(now_ms);
  test.on_loop(now_ms);
  ::nc::monitor_.on_loop(now_ms);
}
