
//#define nc_Monitor_state_verbose

namespace nc {

struct Monitor
  : Runnable
{
  bool active_ = false;
  bool start_ = false;
  bool stop_ = false;

  struct Measure
  {
    static constexpr uint32_t timeout_ms = 10;
    uint32_t timeout_time_ms_;
  } measure_;

  // FIXME: 'pause' -> 'Measure::Period'
  struct Pause
  {
    uint32_t period_ms_ = 5000;
    uint32_t end_time_ms_;
  } pause_;

  enum class State : uint8_t {
    idle,
    start,
    stop,
    measure_init,
    measure_wait,
    measure_done,
    measure_report,
    measure_timeout,
    pause_init,
    pause_wait,
    pause_done,
  };
  State state_ = State::idle;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_idle(uint32_t now_ms);
  void on_state_start(uint32_t now_ms);
  void on_state_stop(uint32_t now_ms);
  void on_state_measure_init(uint32_t now_ms);
  void on_state_measure_wait(uint32_t now_ms);
  void on_state_measure_done(uint32_t now_ms);
  void on_state_measure_report(uint32_t now_ms);
  void on_state_measure_timeout(uint32_t now_ms);
  void on_state_pause_init(uint32_t now_ms);
  void on_state_pause_wait(uint32_t now_ms);
  void on_state_pause_done(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  void start() { start_ = true; }
  void stop() { stop_ = true; }

  struct
  {
    bool raw_ = false;
    bool I_ = false;
    bool V_ = false;
    bool T_ = false;
  } disable_;

  void report() const;
  void report_VT(unsigned spi_sensor_n) const;
};

extern Monitor monitor_;

} // namespace nc
