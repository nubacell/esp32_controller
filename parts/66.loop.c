
void loop()
{
  //LOG_FUNCTION

  static uint32_t loop_count = 0;

  uint32_t now_ms = millis();

  //psDevice.identify();
  //plDevice.identify();

  //cnDevice.poll();
  //psDevice.poll();
  //plDevice.poll();

  uartDevices_on_loop(now_ms);

  if (psDevice.state == PxDevice::State::idle
  &&  plDevice.state == PxDevice::State::idle) {
    LOG_FUNCTION
    delay(15000);
  }

  ++loop_count;
}
