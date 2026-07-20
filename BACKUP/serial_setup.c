
#if 0
void serial_setup()
{
  //Serial.begin(115200);
  xx_uartConfigs_init();
  LOG_FUNCTION
  LOG("uxTaskPriorityGet()=%i", uxTaskPriorityGet(NULL));

  LOG_FUNCTION

#if 0
  Serial.println(ps_uart_cfg.tag);
  xx_uart_cfg_hw_dump(&ps_uart_cfg);
  Serial.println(pl_uart_cfg.tag);
  xx_uart_cfg_hw_dump(&pl_uart_cfg);
  LOG_FUNCTION
#endif
}
#endif
