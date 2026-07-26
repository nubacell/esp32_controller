
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

    if (!spiDevice.areSensorsDataValid()) {
      LOG("ERROR: received sensors' data are invalid");
      state(State::abort);
    } else {
#if 0
      uint64_t us_old = 0;
      uint32_t ms_old = 0;
      for (unsigned i = 0; i < 100; ++i) {
        const auto us = esp_timer_get_time(); // uint64_t
        const auto ms = millis();
        const auto availableForWrite = cn_uartConfig.availableForWrite();
        LOG("%9" PRIu64 " %6u %3u %9" PRIu64 " %6u %4u", us, ms, i, us-us_old, ms-ms_old, availableForWrite);
        us_old = us;
        ms_old = ms;
      }
#endif
      // report sensors' data
      //SensorDevices_Data_report_all();
      rpi::SensorDevices_Data_report_all(/*Test::*/start_time_ms_);

      // start next sub step
      state(State::sub_step_next);

    }
  } else
  if (spi_timeout_time_ms_ <= now_ms) {
    LOG("ERROR: sensors' data SPI transfer timeout");
    state(State::abort);
  }
}
