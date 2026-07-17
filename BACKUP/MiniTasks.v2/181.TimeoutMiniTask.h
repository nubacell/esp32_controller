
struct TimeoutMiniTask
  : MiniTask
{
  uint32_t timeout_time_ms_;
  bool timeout_ = false;

  bool timeout() const { return timeout_; }

  void init(
      MiniTaskObserver miniTaskObserver,
      uint32_t timeout_time_ms)
  {
    MiniTask::init(miniTaskObserver);
    timeout_time_ms_ = timeout_time_ms;
    timeout_ = false;
  }

  void on_loop(uint32_t now_ms)
  {
    if (timeout_time_ms_ <= now_ms) {
      timeout_ = true;
      onTaskFinished();
    }
  }

  static void on_loop_proxy(TimeoutMiniTask * timeoutMiniTask, uint32_t now_ms)
  {
    timeoutMiniTask->on_loop(now_ms);
  }
};
