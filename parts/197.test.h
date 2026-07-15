
struct Test
  : Runnable
{
  unsigned step_;
  unsigned sub_step_;
  //uint32_t start_time_ms_;
  uint32_t event_time_ms_;

  bool run_;
  bool abort_;

  enum class State {
    init,
    idle,
    running,
  };
  static const char * to_string(State state);

  void state(State new_state)
  {
    const auto old_state_name = to_string(state_);
    state_ = new_state;
    const auto new_state_name = to_string(state_);
    LOG("'%s' -> '%s'", old_state_name, new_state_name);
  }

  State state_ = State::init;

  void on_state_init(uint32_t now_ms)
  {
    state(State::idle);
  }


  void change_PS_EL_configs(const ProfileStep * profileStep);
  void deactivate_PS_EL();
  void on_event(uint32_t now_ms);
  void on_loop(uint32_t now_ms) override;
  void on_state_idle(uint32_t now_ms);
  void on_state_running(uint32_t now_ms);

  void run()
  {
    run_ = true;
  }

  void abort()
  {
    abort_ = true;
  }
};
