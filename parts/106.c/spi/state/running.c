
/*
    'spi_transfer_finished_' marks end of single transfer.

    TODO: Implement some state for multiple transfers to integrate them with
    'Test' running.
*/
void SPI_Device::on_state_running(uint32_t now_ms)
{
  if (spi_transfer_finished_) {
    ++spi_sensor_n_;
    spi_sensor_n_ = ::nc::spi::sensors::skip_offline(spi_sensor_n_);
    //LOG("spi_sensor_n_=%u", spi_sensor_n_);
    if (spi_sensor_n_ < SPI_SENSORS_COUNT) {
      if (!spi_transfer_start(spi_sensor_n_)) {
        //scope_sync_on(false);
        state(State::idle);
      }
    } else {
      ::nc::spi::sensors::validate();
      //::nc::spi::sensors::crc_valid_dump();
      //::nc::spi::sensors::valid_dump();
      //::nc::spi::sensors::stats_dump();

      transfer_end_time_ms_ = now_ms;
      //scope_sync_on(false);
      state(State::idle);
    }
  } else
  if (timeout_time_ms_ <= now_ms) {
    LOG("WARNING: %s %s", "spi transfer", "timeout");

    // make sure MR pin is inactive
    gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

    //scope_sync_on(false);
    state(State::run_failed);
  }
}
