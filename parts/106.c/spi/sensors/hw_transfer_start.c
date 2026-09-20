
namespace nc {
namespace spi {

/*
    Warning: 'spi_slave_queue_trans()' does not make copy of
    'spi_slave_transaction_t'. Instead it keeps it's address and uses it to
    store number of actually transferred bits into 'trans_len' field. It means
    that memory occupied by 'spi_slave_transaction_t' must be preserved until
    the end of transmission which is notified by 'post_trans_cb' callback.
*/
//static
//spi_slave_transaction_t Sensors::hw::transaction_;

bool Sensors::hw_transfer_start()
{
  sensors_[sensor_n_].raw.reset();
  spi_mr_mux(sensor_n_);

  if (!hw.chip_.init()) { return false;}

  hw.transaction_ = spi_slave_transaction_t {
    //.flags = 0,
    .length = SPI_DATA_SIZE * 8, // number of bits not bytes
    // trans_len -- used by driver to return actual number of received bits
    //.trans_len = 0,
    .tx_buffer = sensors_[sensor_n_].raw.tx_buffer,
    .rx_buffer = sensors_[sensor_n_].raw.rx_buffer,
    // void * user // User-defined variable.
    .user = this,
  };
  const auto r =
      (ESP_OK == spi_slave_queue_trans(SPI_DEVICE, &hw.transaction_, 0));
  hw.active_ = r;
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_queue_trans");
    hw.chip_.down();
  }
  return r;
}

} // namespace spi
} // namespace nc
