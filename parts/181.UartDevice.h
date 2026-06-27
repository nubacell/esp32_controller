
#define udLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct UartDevice
  : Runnable
{
  const UartConfig * uartConfig_;

  size_t buffer_used_ = 0;
  static constexpr size_t buffer_size = 64;
  byte buffer_[buffer_size];
  byte last_char_;
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
#if 1
        if ('\r' == last_char_ && '\n' == b) { // CR+LF
          last_char_ = 0; // intentionally store not '\n'
        } else
        if ('\n' == last_char_ && '\r' == b) { // LF+CR
          last_char_ = 0; // intentionally store not '\r'
        } else
#endif
        if ('\n' == b || '\r' == b) { // CR || LF
          eol_ = true;
          // last_char_ will be used on next byte to detect CR+LF and LF+CR
          last_char_ = b;
          break;
        } else
        // reserve space for trailing '\0'
        if ((buffer_used_ + 1) < sizeof(buffer_)) {
          buffer_[buffer_used_++] = b;
          buffer_[buffer_used_] = '\0';
          last_char_ = b;
        } else {
          overflow_ = true;
          last_char_ = b;
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
