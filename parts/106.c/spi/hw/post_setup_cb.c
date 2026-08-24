
// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
unsigned spi_post_setup_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_setup_cb_us_;
#endif
void spi_post_setup_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_setup_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_setup_cb_us_ = esp_timer_get_time();
#endif

  spi_mr_mux(spi_sensor_n_);

  //spi_sensor_n_ %= SPI_SENSORS_COUNT;
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_ACTIvE);
}
