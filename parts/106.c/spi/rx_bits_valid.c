
bool spi_rx_bits_valid()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    if (spi_rx_bits[i] < SPI_RX_BITS_MIN) {
      //LOG(" . spi_rx_bits[%u]=%u SPI_RX_BITS_MIN=%u", i, spi_rx_bits[i], SPI_RX_BITS_MIN);
      return false;
    }
  }
  return true;
}
