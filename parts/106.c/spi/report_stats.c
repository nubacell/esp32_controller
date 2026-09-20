
void spi_report_stats()
{
  LOG(". post_setup_cb_count=%u post_trans_cb_count=%u",
      ::nc::spi::sensors.stats.post_setup_cb_count,
      ::nc::spi::sensors.stats.post_trans_cb_count);

  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    hex_dump(::nc::spi::sensors.sensors_[i].raw.rx_buffer, SPI_DATA_SIZE);
  }
}
