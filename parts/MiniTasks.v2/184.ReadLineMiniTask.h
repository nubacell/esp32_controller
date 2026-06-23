
struct ReadLineMiniTask
  : TimeoutMiniTask
  , UartMiniTask
{
  byte * buffer_;
  size_t buffer_size_;
  size_t data_size_ = 0;
  bool overflow_ = false;
  bool eol_ = false;

  bool overflow() const { return overflow_; }
  bool eol() const { return eol_; }

  void init(
      MiniTaskObserver miniTaskObserver,
      uint32_t timeout_time_ms,
      UartDevice * uartDevice,
      byte * buffer, size_t buffer_size)
  {
    TimeoutMiniTask::init(miniTaskObserver, timeout_time_ms);
    UartMiniTask::init(uartDevice);
    buffer_ = buffer;
    buffer_size_ = buffer_size;

    if (buffer_ && buffer_size_) {
      // trailing '\0' for convenience
      buffer_[0] = '\0';
    }
  }

  void on_loop(uint32_t now_ms)
  {
    while (availableForRead()) {
      const auto b = read_byte(0);
      if ('\n' == b) {
        eol_ = true;
        onTaskFinished();
        return;
      } else {
        // reserve space for trailing '\0'
        if (buffer_ && (data_size_ + 1) < buffer_size_) {
          buffer_[data_size_++] = b;
          buffer_[data_size_] = '\0';
        } else {
          overflow_ = true;
        }
      }
    }

    TimeoutMiniTask::on_loop(now_ms);
  }

  static void on_loop_proxy(ReadLineMiniTask * readLineMiniTask, uint32_t now_ms)
  {
    readLineMiniTask->on_loop(now_ms);
  }
};
