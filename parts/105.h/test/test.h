
struct Test
  : Runnable
{
  uint32_t test_start_time_ms_;

  struct Step
  {
    unsigned count_;
    uint32_t start_time_planned_ms_;
    uint32_t start_time_actual_ms_;
    uint32_t end_time_planned_ms_;
  } step_;
  static constexpr unsigned step_count_max_ = 100;

  struct Sub
  {
    Step step_;
  } sub_;
  static constexpr unsigned sub_step_count_max_ = 10000;

  struct Measure
  {
    uint32_t start_time_actual_ms_;
    uint32_t end_time_actual_ms_;

    // FIXME: Use correct 'Measure::timeout_ms'
    static constexpr uint32_t timeout_ms = 10;
    uint32_t timeout_time_ms_;
  } measure_;

  int32_t I_A = 0;
  uint32_t PS_V_max = 0;

  bool run_;
  bool abort_;

  enum class State : uint8_t {
    idle,
//  init,                   // start_time_ms_ = sub_step_end_time_ms_ = now_ms;
//  step_0,                 // step_ = 0;
    step_init,              // 1 set PS/EL configs
//       sub_step_0,        // sub_step_ = 0;
         sub_step_init,     // 2 'sub_step_end_time_ms_ += measure_period_ms_;'
         sub_step_wait,     // 3 wait for end of sub step (sub_step_end_time_ms_)
           measure_init,
//           spi_init,      //   start SPI transfer to get measurements
//           i2c_adc_init,  //   start i2c ADC measurements
           measure_wait,    //   wait for end of SPI transfer and i2c ADC measurements
         sub_step_next,     // 5 '++sub_step_ < substeps_count_ ? sub_step_init : step_next'
    step_next,              // 6 '++step_ < steps_count_ ? step_init : cleanup'
    abort,                  // 7 report and cleanup (deactivate PS and EL)
    cleanup,                // 8 deactivate PS and EL
  };
  State state_ = State::idle;
  static const char * to_string(State state);

  void state(State new_state);

  void change_PS_EL_configs(const ProfileStep * profileStep);
  void deactivate_PS_EL();

  void on_loop(uint32_t now_ms) override;

  void on_state_idle(uint32_t now_ms);
  void on_state_step_init(uint32_t now_ms);
  void on_state_sub_step_init(uint32_t now_ms);
  void on_state_sub_step_wait(uint32_t now_ms);
  void on_state_measure_init(uint32_t now_ms);
  void on_state_measure_wait(uint32_t now_ms);
  void on_state_sub_step_next(uint32_t now_ms);
  void on_state_step_next(uint32_t now_ms);
  void on_state_abort(uint32_t now_ms);
  void on_state_cleanup(uint32_t now_ms);

  void run()
  {
    run_ = true;
  }

  void abort()
  {
    abort_ = true;
  }

  void dump() const;

  void report_step_data(
      uint32_t now_ms,
      uint32_t test_start_time_ms) const;

};

extern Test test;
