
namespace nc {
namespace spi {
namespace hw {

void Chip::down()
{
  if (!active_) { return; }

  const auto r = (ESP_OK == spi_slave_free(SPI_DEVICE));
  if (!r) {
    LOG("ERROR: '%s()' failure", "spi_slave_free");
  }
  active_ = false;
}

} // namespace hw
} // namespace spi
} // namespace nc
