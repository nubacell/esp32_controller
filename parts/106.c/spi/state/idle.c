
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

    spi_sensor_n_ = ::nc::spi::sensors::skip_offline(spi_sensor_n_);
    //LOG("spi_sensor_n_=%u", spi_sensor_n_);
    if (spi_sensor_n_ >= SPI_SENSORS_COUNT) {
      state(State::idle);
      return;
    }

    memset(::nc::spi::buffers::rx, 'R', sizeof(::nc::spi::buffers::rx));
    //memset(::nc::spi::buffers::tx, 'T', sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0x00, sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0x0F, sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0xFF, sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0xC3, sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0xF0, sizeof(::nc::spi::buffers::tx));
    //memset(::nc::spi::buffers::tx, 0x81, sizeof(::nc::spi::buffers::tx));
    memset(::nc::spi::buffers::tx, 0x99, sizeof(::nc::spi::buffers::tx));

    if (spi_transfer_start(spi_sensor_n_)) {
      state(State::running);
    } else {
      //scope_sync_on(false);
      state(State::idle);
    }
  }
}
