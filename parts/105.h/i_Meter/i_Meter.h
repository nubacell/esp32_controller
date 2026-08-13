
#define I_METER_CHANNELS_COUNT 2

struct I_Meter
  : Runnable
{
  uint64_t start_time_us_;
  uint64_t finish_time_us_;

  uint32_t start_time_ms_;
  uint32_t finish_time_ms_;
  uint32_t timeout_time_ms_;

  bool run_ = false;
  bool measurementsValid_ = false;

  enum class State {
    init,
    init_failed,
    idle,
    running,
    run_failed,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  static constexpr uint32_t single_conversion_timeout_ms = 5;

  uint16_t conversion_results[I_METER_CHANNELS_COUNT];
  uint8_t channel_;

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_running(uint32_t now_ms);
  void on_state_run_failed(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  void run()
  {
    run_ = true;
  }

  bool isMeasurementFinished() const { return State::idle == state_; }
  bool areMeasurementsValid() const { return measurementsValid_; }
  void reportMeasurements(
      uint32_t now_ms,
      uint32_t test_start_time_ms) const;
};

extern I_Meter iMeter;
