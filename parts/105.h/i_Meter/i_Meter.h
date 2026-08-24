
#define I_METER_CHANNELS_COUNT 2

struct I_Meter
  : Runnable
{
  uint32_t start_time_ms_;
  uint32_t end_time_ms_;

  static constexpr uint32_t single_conversion_timeout_ms = 5;
  uint32_t timeout_time_ms_;

  bool run_ = false;
  bool measurementsValid_ = false;

  enum class State : uint8_t {
    init,
    init_failed,
    idle,
    running,
    run_failed,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  uint16_t conversion_results[I_METER_CHANNELS_COUNT];
  uint8_t channel_;

  struct Stats {
    unsigned start_;
    unsigned start_timeout_;
    unsigned poll_;
    unsigned poll_timeout_;
    unsigned read_;
    unsigned read_timeout_;
  } stats_[I_METER_CHANNELS_COUNT];

  bool start();
  bool poll(bool & ready);
  bool read(uint16_t & v);

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

  bool isMeasurementFinished() /*const*/ { return State::idle == state_; }
  bool areMeasurementsValid() /*const*/ { return measurementsValid_; }
  void reportMeasurements(
    ::nc::mk::Buffer & b,
    uint32_t now_ms,
    uint32_t test_start_time_ms) const;
  void reportMeasurements(
      uint32_t now_ms,
      uint32_t test_start_time_ms) const;
  void reportStats() const;
  void reportStats(::nc::mk::Buffer & b) const;
};

extern I_Meter iMeter;
