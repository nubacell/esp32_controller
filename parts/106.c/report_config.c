
void report_config()
{
  Serial.print("device: '"); Serial.print(device_tag); Serial.println("'");
  ::nc::spi::report_build_config();
}
