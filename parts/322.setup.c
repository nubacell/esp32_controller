
void setup()
{
  xx_uartConfigs_init();

  // move 'spi_setup()' to 'spiDevice::on_state_init()'
  //spi_setup();

  report_config();

  //STACK_MARKER(256);
}
