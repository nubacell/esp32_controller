
/*
    Warning: 'spi_slave_queue_trans()' does not make copy of
    'spi_slave_transaction_t'. Instead it keeps it's address and uses it to
    store number of actually transferred bits into 'trans_len' field. It means
    that memory occupied by 'spi_slave_transaction_t' must be preserved until
    the end of transmission which is notified by 'post_trans_cb' callback.
*/
static spi_slave_transaction_t spi_transaction_;

bool spi_transfer_start(uint8_t spi_sensor_n)
{
  spi_transfer_finished_ = false;

  spi_transaction_ = spi_slave_transaction_t {
    //.flags = 0,
    .length = SPI_DATA_SIZE * 8, // number of bits not bytes
    // trans_len -- used by driver to return actual number of received bits
    //.trans_len = 0,
    .tx_buffer = &::nc::spi::buffers::tx[spi_sensor_n][0],
    .rx_buffer = &::nc::spi::buffers::rx[spi_sensor_n][0],
    // void * user // User-defined variable.
    // Can be used to store 'spi_sensor_n' for 'spi_post_setup_cb' and 'spi_post_trans_cb'.
    //.user = NULL,
  };
  if (ESP_OK != spi_slave_queue_trans(SPI_DEVICE, &spi_transaction_, 0)) {
    LOG("ERROR: %s() failure", "spi_slave_queue_trans");
    return false;
  }
  return true;
}
