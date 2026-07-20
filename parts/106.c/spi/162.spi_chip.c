
bool spi_chip_active_ = false;

bool spi_chip_init()
{
  LOG("");
  if (spi_chip_active_) { return true; }

  const auto r = (ESP_OK == spi_slave_initialize(SPI_DEVICE,
      &spi_bus_config, &spi_slave_interface_config, SPI_DMA_DISABLED));
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_initialize");
  } else {
    spi_chip_active_ = true;
  }
  return r;
}

bool spi_chip_down()
{
  if (!spi_chip_active_) { return true; }

  const auto r = (ESP_OK == spi_slave_free(SPI_DEVICE));
  if (!r) {
    LOG("ERROR: '%s()' failure", "spi_slave_free");
  } else {
    spi_chip_active_ = false;
  }
  return r;
}
