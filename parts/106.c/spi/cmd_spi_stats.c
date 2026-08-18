
void spi_stats(const char * args)
{
  (void) args;
  spi_report_stats();
  SensorDevices_Data_report_all();
}
