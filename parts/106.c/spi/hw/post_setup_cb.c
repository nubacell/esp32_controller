
// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
void spi_post_setup_cb(spi_slave_transaction_t * transaction)
{
  auto sensors = static_cast<::nc::spi::Sensors*>(transaction->user);
  ++sensors->stats.post_setup_cb_count;

  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_ACTIvE);
}
