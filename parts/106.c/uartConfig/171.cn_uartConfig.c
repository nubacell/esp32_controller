
const UartConfig cn_uartConfig = {
  .tag =  "CN-u" MAKE_STRING(CN_UART_NUM),
  .hs = &Serial,
  .baud = CN_UART_BAUD,
  .n = 0,
  .rxd_gpio = -1, // '-1' == 'do not change'
  .txd_gpio = -1, // '-1' == 'do not change'
//.rxBufferSize = 0,
  .rxBufferSize = 0x400,
  .txBufferSize = 0x800,
//.txBufferSize = 0x1000,
};
