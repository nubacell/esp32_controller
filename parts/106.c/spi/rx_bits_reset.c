
void spi_rx_bits_reset()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    spi_rx_bits[i] = 0;
  }
}
