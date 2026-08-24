
struct SPI_Device
  : Runnable
{
  uint32_t transfer_start_time_ms_;
  uint32_t transfer_end_time_ms_;
  uint32_t timeout_time_ms_;

  bool run_ = false;

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

  bool isTransferFinished() { return State::idle == state_; }
  bool areSensorsDataValid() { return ::nc::spi::sensors::valid_all_; }
};

extern SPI_Device spiDevice;
