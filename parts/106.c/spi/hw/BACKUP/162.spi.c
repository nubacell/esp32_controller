
//#define SPI_TRANS_TIME_STATS


#if 0
bool spi_init()
{
  //LOG("");
  //const auto r = spi_chip_init();
  const auto r = true;
  //LOG("");
  return r;
}

bool spi_down()
{
  //LOG("");

#if 0
  const auto spi_slave_disable_r = (ESP_OK == spi_slave_disable(SPI_DEVICE));
  if (!spi_slave_disable_r) {
    LOG("ERROR: '%s()' failure", "spi_slave_disable");
  }
  //LOG("");
#endif

#if 1
  const auto r = true;
#else
  const auto r = spi_chip_down();

  gpio_set_level(gpio_num_t(SPI_GPIO_MISO), 1);
  gpio_set_pull_mode(gpio_num_t(SPI_GPIO_MISO), GPIO_PULLUP_ONLY);
  gpio_set_direction(gpio_num_t(SPI_GPIO_MISO), GPIO_MODE_INPUT_OUTPUT);
#endif

  //LOG("");
  return r;
}
#endif
