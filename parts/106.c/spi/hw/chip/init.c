
namespace nc {
namespace spi {
namespace hw {

bool Chip::init()
{
  //LOG("");
  if (active_) { return true; }

  const auto r = (ESP_OK == spi_slave_initialize(SPI_DEVICE,
      &spi_bus_config, &spi_slave_interface_config, SPI_DMA_DISABLED));
  active_ = r;
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_initialize");
  }
  return r;
}

} // namespace hw
} // namespace spi
} // namespace nc
