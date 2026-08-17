
// Long running state. Check for user's abort request.

// 'measure_wait'
void Test::on_state_measure_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (spiDevice.isTransferFinished() && iMeter.isMeasurementFinished()) {
    //LOG("'transferFinished'");

    // --- 'measure end' ---

    if (!spiDevice.areSensorsDataValid()) {
      LOG("ERROR: received sensors' data are invalid");
      state(State::abort);
    } else if (!iMeter.areMeasurementsValid()) {
      LOG("ERROR: 'current' measurements are invalid");
      state(State::abort);
    } else {
      // report sensors' data
      iMeter.reportMeasurements(now_ms, test_start_time_ms_);
      rpi::SensorDevices_Data_report_all(now_ms, test_start_time_ms_);
      report_step_data(now_ms, test_start_time_ms_);

      // start next sub step
      state(State::sub_step_next);
    }
  } else
  if (measure_timeout_time_ms_ <= now_ms) {
    LOG("ERROR: measurements timeout");
    state(State::abort);
  }
}
