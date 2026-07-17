
static void report_config()
{
  Serial.print("device: '"); Serial.print(device_tag); Serial.println("'");
  spi_report_config();
}
