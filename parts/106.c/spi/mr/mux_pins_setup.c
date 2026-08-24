
void spi_mr_mux_pins_setup()
{
  LOG("");
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, 0);
    gpio_set_direction(spi_mr_mux_pin, GPIO_MODE_INPUT_OUTPUT);
  }
}
