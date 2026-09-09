
static const UartConfig el_uartConfig = {
  .tag = "EL-u" MAKE_STRING(EL_UART_NUM),
  .hs = &Serial2,
  .baud = EL_UART_BAUD,
  .rxBufferSize = 0,
  .txBufferSize = 0,
  .n = 2,
  .rxd_gpio = EL_UART_RX_GPIO,
  .txd_gpio = EL_UART_TX_GPIO,
};
