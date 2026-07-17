
struct LineReader
  : Runnable
{
  const UartConfig * uartConfig_;

  Callback eolCallback_ = {};
  Callback overflowCallback_ = {};

  byte * buffer_;
  size_t buffer_size_;

  size_t data_size_ = 0;

  bool overflow_ = false;
  bool eol_ = false;

  bool overflow() const { return overflow_; }
  bool eol() const { return eol_; }

  void reset()
  {
    data_size_ = 0;

    overflow_ = false;
    eol_ = false;

    if (buffer_ && buffer_size_) {
      // trailing '\0' for convenience
      buffer_[0] = '\0';
    }
  }

  LineReader(
      const UartConfig * uartConfig,
      Callback eolCallback,
      Callback overflowCallback,
      byte * buffer,
      size_t buffer_size
  )
    : uartConfig_(uartConfig)
    , eolCallback_(eolCallback)
    , overflowCallback_(overflowCallback)
    , buffer_(buffer)
    , buffer_size_(buffer_size)
  {
    reset();
  }

  void on_eol()
  {
    eol_ = true;
    eolCallback_(this);
  }

  void on_overflow()
  {
    overflow_ = true;
    overflowCallback_(this);
  }

  void on_loop(uint32_t now_ms) override
  {
    auto available = uartConfig_->availableForRead();
    while (available--) {
      const auto b = uartConfig_->read_byte(0);
      if ('\n' == b) {
        on_eol();
        reset();
        break;
      } else {
        // reserve space for trailing '\0'
        if (buffer_ && (data_size_ + 1) < buffer_size_) {
          buffer_[data_size_++] = b;
          buffer_[data_size_] = '\0';
        } else {
          on_overflow();
          overflowCallback_(this);
          reset();
        }
      }
    }
  }
};
