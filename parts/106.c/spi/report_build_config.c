
namespace nc {
namespace spi {

void report_build_config()
{
  LOG(". %s=%u", "SPI_GPIO_MISO", SPI_GPIO_MISO);
  LOG(". %s=%u", "SPI_GPIO_MOSI", SPI_GPIO_MOSI);
  LOG(". %s=%u", "SPI_GPIO_SCLK", SPI_GPIO_SCLK);
  LOG(". %s=%u", "SPI_GPIO_CS", SPI_GPIO_CS);
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = spi_mr_mux_pins[i];
    LOG(". SPI_MR_MUX_%u_GPIO=%u", i, spi_mr_mux_pin);
  }
  LOG(". %s=%u", "SPI_MR_GPIO", SPI_MR_GPIO);
  LOG(". %s=%u", "SPI_BUFFER_SIZE", SPI_BUFFER_SIZE);
  LOG(". %s=%u", "SPI_DATA_SIZE", SPI_DATA_SIZE);
  LOG(". %s=%u", "SPI_RX_BITS_MIN", SPI_RX_BITS_MIN);
}

} // namespace spi
} // namespace nc
