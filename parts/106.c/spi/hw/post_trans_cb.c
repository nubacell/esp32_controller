
bool spi_transfer_finished_;

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
unsigned spi_post_trans_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_trans_cb_us_;
uint32_t spi_post_trans_cb_us_min_ = uint32_t(-1);
uint32_t spi_post_trans_cb_us_max_;
#endif
void spi_post_trans_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_trans_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_trans_cb_us_ = esp_timer_get_time();
  uint32_t trans_us = spi_post_trans_cb_us_ - spi_post_setup_cb_us_;
  if (spi_post_trans_cb_us_min_ > trans_us) {
    spi_post_trans_cb_us_min_ = trans_us;
  }
  if (spi_post_trans_cb_us_max_ < trans_us) {
    spi_post_trans_cb_us_max_ = trans_us;
  }
#endif

  //spi_sensor_n_ %= SPI_SENSORS_COUNT;
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  spi_rx_bits[spi_sensor_n_] = transaction->trans_len;

  spi_transfer_finished_ = true;
}
