
// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of transfer.
void spi_post_trans_cb(spi_slave_transaction_t * transaction)
{
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  auto sensors = static_cast<::nc::spi::Sensors*>(transaction->user);
  ++sensors->stats.post_trans_cb_count;
  sensors->hw.active_ = false;
}
