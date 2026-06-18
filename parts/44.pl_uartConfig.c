
static const UartConfig pl_uartConfig = {
  .tag = "PL-u" MAKE_STRING(PL_UART_NUM),
  .hs = &Serial2,
  .baud = PL_UART_BAUD,
  .n = 2,
  .rxd_gpio = PL_UART_RX_GPIO,
  .txd_gpio = PL_UART_TX_GPIO,
};
