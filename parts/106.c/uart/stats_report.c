
void uart_stats_report()
{
  auto & buffer = ::nc::shared_buffers.uart_stats_buffer;
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/STATS/UART rx{available.max/FIFO/BUFFER/PARITY/FRAME} tx{available.{min/max}}");

  static const struct {
    const char * const tag;
    const UartDevice * const uartDevice;
  } uartDevices[] = {
    { "cn", &cnDevice, },
    { "ps", &psDevice, },
    { "el", &elDevice, },
  };;
  for (unsigned i = 0; i < ARRAY_SIZE(uartDevices); ++i) {
    b.printf(" %s %zi/%u/%u/%u/%u %zi/%zi", uartDevices[i].tag,
      uartDevices[i].uartDevice->stats_.availableForRead_max_,
      uartDevices[i].uartDevice->stats_.UART_FIFO_OVF_ERROR_,
      uartDevices[i].uartDevice->stats_.UART_BUFFER_FULL_,
      uartDevices[i].uartDevice->stats_.UART_PARITY_ERROR_,
      uartDevices[i].uartDevice->stats_.UART_FRAME_ERROR_,
      uartDevices[i].uartDevice->stats_.availableForWrite_min_,
      uartDevices[i].uartDevice->stats_.availableForWrite_max_
    );
  }

  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}
