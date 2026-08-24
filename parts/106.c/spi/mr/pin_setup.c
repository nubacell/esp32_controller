
void spi_mr_pin_setup()
{
  LOG("");
  const auto spi_mr_pin = gpio_num_t(SPI_MR_GPIO);
  gpio_set_level(spi_mr_pin, 1);
  gpio_set_direction(spi_mr_pin, GPIO_MODE_INPUT_OUTPUT);
}
