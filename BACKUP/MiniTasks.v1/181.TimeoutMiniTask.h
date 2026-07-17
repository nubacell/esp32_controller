
struct TimeoutMiniTask
  : MiniTask
{
  uint32_t timeout_time_ms_;
  bool timeout_ = false;

  bool timeout() const { return timeout_; }

  TimeoutMiniTask(
      MiniTaskObserver * miniTaskObserver,
      uint32_t timeout_time_ms)
    : MiniTask(miniTaskObserver)
    , timeout_time_ms_(timeout_time_ms)
  {}

  void on_loop(uint32_t now_ms) override
  {
    if (timeout_time_ms_ <= now_ms) {
      timeout_ = true;
      onTaskFinished();
    }
  }
};
