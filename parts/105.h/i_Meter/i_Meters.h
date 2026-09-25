
//#define iMeters_state_verbose

#define I_METERS_COUNT 2

struct I_Meters
  : Runnable
{
  struct Stats {
    ::nc::stats::OkNg start;
    ::nc::stats::OkNg poll;
    ::nc::stats::OkNg read;

    static constexpr const char * const tags[] = {
      "start",
      "poll",
      "read",
    };

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } stats;

  uint32_t start_time_ms_;
  uint32_t end_time_ms_;
  static constexpr uint32_t single_conversion_timeout_ms = 5;
  uint32_t timeout_time_ms_;

  I_Meter iMeters_[I_METERS_COUNT];

  uint8_t channel_;

  bool adc_start();
  bool adc_poll(bool & ready);
  bool adc_read(uint16_t & v);

  enum class State : uint8_t {
    init,
    init_failed,
    idle,
    adc_init,
    adc_wait,
    adc_timeout,
    next_channel,
    run_finished,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_adc_init(uint32_t now_ms);
  void on_state_adc_wait(uint32_t now_ms);
  void on_state_adc_timeout(uint32_t now_ms);
  void on_state_next_channel(uint32_t now_ms);
  void on_state_run_finished(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  bool run_ = false;
  inline void run() { run_ = true; }
  bool run_finished_ = false;
  inline bool run_finished() /*const*/ { return run_finished_; }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

extern I_Meters iMeters;

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meters::Stats& stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meters & iMeters) noexcept
{
  return iMeters.to_buffer(b);
}
