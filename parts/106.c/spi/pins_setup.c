
#if 0
// GPIO_DRIVE_CAP_0: ~5 mA
// GPIO_DRIVE_CAP_1: ~10 mA
// GPIO_DRIVE_CAP_2: ~20 mA -- default
// GPIO_DRIVE_CAP_3: ~40 mA
void spi_pins_setup()
{
  LOG("");
  // MISO pin (if used) is 'OUTPUT'.
  //pinMode(SPI_GPIO_MISO, INPUT_PULLUP);
  //pinMode(SPI_GPIO_MISO, OUTPUT);
  //digitalWrite(SPI_GPIO_MISO, HIGH);

  // MOSI, SCLK and CS pins are 'INPUT_PULLUP'
  for (unsigned i = 0; i < ARRAY_SIZE(spi_pins); ++i) {
    uint8_t spi_pin = spi_pins[i];
    pinMode(spi_pin, INPUT_PULLUP);
    //gpio_set_pull_mode(spi_pin, GPIO_PULLUP_ONLY);
    //gpio_set_drive_capability(spi_pin, GPIO_DRIVE_CAP_3);
  }
}
#endif
