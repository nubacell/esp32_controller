
const UartConfig cn_uartConfig = {
  .tag =  "CN-u" MAKE_STRING(CN_UART_NUM),
  .hs = &Serial,
  .baud = CN_UART_BAUD,

//.rxBufferSize = 0,
//.rxBufferSize = 0x200,
//.rxBufferSize = 0x400,
  .rxBufferSize = 0x4000,

  .txBufferSize = 0x800,
//.txBufferSize = 0x1000,

  .n = 0,
  .rxd_gpio = -1, // '-1' == 'do not change'
  .txd_gpio = -1, // '-1' == 'do not change'

//.rx_fifo_irq_bytes = 60,
  .rx_fifo_irq_bytes = 110, // ??
//.rx_fifo_irq_bytes = 100, // OK
};
