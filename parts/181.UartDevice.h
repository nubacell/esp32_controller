
#define udLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct UartDevice
  : Runnable
{
  const UartConfig * uartConfig_;

  static constexpr size_t buffer_size = 64;
  byte buffer_[buffer_size];
  size_t buffer_used_ = 0;
  bool eol_ = false;
  bool overflow_ = false;

  UartDevice(const UartConfig * uartConfig)
    : uartConfig_(uartConfig)
  {}

  // assuming 'sizeof(buffer_) > 0'
  void on_loop_readLine_reset()
  {
    buffer_used_ = 0;
    overflow_ = false;
    eol_ = false;
    buffer_[0] = '\0'; // trailing '\0' for convenience
  }

  // returns '(eol_ || overflow_)'
  bool on_loop_readLine()
  {
    if (!eol_ && !overflow_) {
      auto available = uartConfig_->availableForRead();
      while (available--) {
        const auto b = uartConfig_->read_byte(0);
        if ('\n' == b || '\r' == b) {
          eol_ = true;
          break;
        } else
        // reserve space for trailing '\0'
        if ((buffer_used_ + 1) < sizeof(buffer_)) {
          buffer_[buffer_used_++] = b;
          buffer_[buffer_used_] = '\0';
        } else {
          overflow_ = true;
          break;
        }
      }
    }
    return eol_ || overflow_;
  }

  void on_loop(uint32_t now_ms) override
  {
    if (on_loop_readLine()) {
      udLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_readLine_reset();
    }
  }
};
