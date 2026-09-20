
namespace nc {
namespace spi {

struct Sensors
  : Runnable
{
  struct Stats
  {
    ::nc::stats::counter_t post_setup_cb_count;
    ::nc::stats::counter_t post_trans_cb_count;

    static constexpr const char * const tags[] = {
      "setup_cb",
      "trans_cb",
    };

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } stats;

  ::nc::mk::Buffer &
  stats_to_buffer(::nc::mk::Buffer & b) const noexcept;

  uint32_t transfer_start_time_ms_;
  uint32_t transfer_end_time_ms_;
  static constexpr uint32_t single_transfer_timeout_ms = 5;
  uint32_t timeout_time_ms_;

  Sensor sensors_[SPI_SENSORS_COUNT];

  struct HW
  {
    ::nc::spi::hw::Chip chip_;
    //static
    spi_slave_transaction_t transaction_;
    bool active_;
    uint16_t rx_bits() const { return transaction_.trans_len; }
  } hw;
  bool hw_transfer_start();

  uint8_t sensor_n_;

  bool offline_[SPI_SENSORS_COUNT];
  void offline_config(uint32_t v);
  void offline_config_dump() const;

  void skip_offline();
  inline bool offline(unsigned sensor_n) const { return offline_[sensor_n]; }

  enum class State : uint8_t {
    init,
    init_failed, // can be used to retry init after some delay
    idle,
    transfer_init,
    transfer_wait,
    transfer_timeout,
    next_sensor,
    run_finished,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_transfer_init(uint32_t now_ms);
  void on_state_transfer_wait(uint32_t now_ms);
  void on_state_transfer_timeout(uint32_t now_ms);
  void on_state_next_sensor(uint32_t now_ms);
  void on_state_run_finished(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  bool run_ = false;
  inline void run() { run_ = true; }
  bool run_finished_ = false;
  inline bool run_finished() const { return run_finished_; }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

extern Sensors sensors;

} // namespace spi
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensors::Stats & stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensors & sensors) noexcept
{
  return sensors.to_buffer(b);
}
