
void spi_report_stats()
{
  LOG(". spi_post_setup_cb_count_=%u spi_post_trans_cb_count_=%u",
      spi_post_setup_cb_count_, spi_post_trans_cb_count_);
#if defined SPI_TRANS_TIME_STATS
  LOG(". spi_post_trans_cb_us_min_=%u spi_post_trans_cb_us_max_=%u",
      spi_post_trans_cb_us_min_, spi_post_trans_cb_us_max_);
#endif

  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    hex_dump(&::nc::spi::buffers::rx[i][0], SPI_DATA_SIZE);
  }
}
