
void spi_mr_mux(unsigned spi_sensor_n)
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, (spi_sensor_n >> i) & 1);
  }
}
