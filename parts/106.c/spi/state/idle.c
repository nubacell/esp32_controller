
void SPI_Device::on_state_idle(uint32_t now_ms)
{
  (void) now_ms;
  if (run_) {
    run_ = false;

    //scope_sync_on(true);
    transfer_start_time_ms_ = now_ms;

    constexpr uint32_t spi_transfer_all_timeout_ms = 100;
    timeout_time_ms_ = now_ms + spi_transfer_all_timeout_ms;

    spi_rx_bits_reset();
    spi_sensor_n_ = 0;
    sensors_data_valid_ = false;

    memset(spi_rx_buffers, 'R', sizeof(spi_rx_buffers));
    //memset(spi_tx_buffers, 'T', sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x00, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x0F, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xFF, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xC3, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xF0, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x81, sizeof(spi_tx_buffers));
    memset(spi_tx_buffers, 0x99, sizeof(spi_tx_buffers));

    if (spi_transfer_start(spi_sensor_n_)) {
      state(State::running);
    } else {
      //scope_sync_on(false);
      state(State::idle);
    }
  }
}
