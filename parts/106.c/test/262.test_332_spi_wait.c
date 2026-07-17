
// Long running state. Check for user's abort request.

// 'spi_wait'
void Test::on_state_spi_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (spiDevice.isTransferFinished()) {
    //LOG("'transferFinished'");

    // --- 'spi transfer end' ---

    // FIXME: temporarily changed '!' to '!!' while SPI is disconnected
    if (!!spiDevice.areSensorsDataValid()) {
      LOG("ERROR: received sensors' data are invalid");
      state(State::abort);
    } else {
      // report sensors' data
      SensorDevices_Data_report_all();

      // start next sub step
      state(State::sub_step_next);

    }
  } else
  if (spi_timeout_time_ms_ <= now_ms) {
    LOG("ERROR: sensors' data SPI transfer timeout");
    state(State::abort);
  }
}
