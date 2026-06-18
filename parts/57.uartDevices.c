static UartDevice * const uartDevices[] = {
  &cnDevice,
  &psDevice,
  &plDevice,
};

static void uartDevices_on_loop(int32_t now_ms) {
  for (unsigned i = 0; i < ARRAY_SIZE(uartDevices); ++i) {
    uartDevices[i]->on_loop(now_ms);
  }
}
