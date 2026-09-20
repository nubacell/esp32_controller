
// Long running state. Check for user's abort request.

// 'measure_wait'
void Test::on_state_measure_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (::nc::spi::sensors.run_finished()
  && iMeter.isMeasurementFinished()) {
    // --- 'measure end' ---
    measure_.end_time_actual_ms_ = now_ms;

    // FIXME: Do not abort test on invalid measurements.

    if (!iMeter.areMeasurementsValid()) {
      LOG("ERROR: 'current' measurements are invalid");
      state(State::abort);
    } else {
      // report measurements
      report_step_data(now_ms, test_start_time_ms_);

      // start next sub step
      state(State::sub_step_next);
    }
  } else
  if (measure_.timeout_time_ms_ <= now_ms) {
    LOG("ERROR: measurements timeout");
    state(State::abort);
  }
}
