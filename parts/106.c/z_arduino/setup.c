
void setup()
{
  xx_uartConfigs_init();
  //scope_sync_setup();
  spi_setup();
  report_config();

  cnDevice.attach_onReceiveError();
  psDevice.attach_onReceiveError();
  elDevice.attach_onReceiveError();
}
