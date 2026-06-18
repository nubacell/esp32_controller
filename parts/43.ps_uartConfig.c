
static const UartConfig ps_uartConfig = {
  .tag =  "PS-u" MAKE_STRING(PS_UART_NUM),
  .hs = &Serial1,
  .baud = PS_UART_BAUD,
  .n = 1,
  .rxd_gpio = PS_UART_RX_GPIO,
  .txd_gpio = PS_UART_TX_GPIO,
};
