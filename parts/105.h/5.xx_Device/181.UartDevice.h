
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

  mutable
  struct Stats
  {
    // rx
    size_t availableForRead_max_ = 0;
    unsigned UART_FIFO_OVF_ERROR_ = 0;
    unsigned UART_BUFFER_FULL_ = 0;
    unsigned UART_PARITY_ERR_ = 0;
    unsigned UART_PARITY_ERROR_ = 0;
    unsigned UART_FRAME_ERROR_ = 0;

    // tx
    size_t availableForWrite_min_ = (size_t)-1;
    size_t availableForWrite_max_ = 0;
  } stats_;

  void onReceiveError(hardwareSerial_error_t e)
  {
    switch (e) {
      case UART_FIFO_OVF_ERROR: ++stats_.UART_FIFO_OVF_ERROR_; break;
      case UART_BUFFER_FULL   : ++stats_.UART_BUFFER_FULL_; break;
      case UART_PARITY_ERROR  : ++stats_.UART_PARITY_ERROR_; break;
      case UART_FRAME_ERROR   : ++stats_.UART_FRAME_ERROR_; break;
      default: break;
    }
  }

  void attach_onReceiveError()
  {
    uartConfig_->hs->onReceiveError(
        [this](hardwareSerial_error_t e)
        {
          this->onReceiveError(e);
        }
    );
  }

  UartDevice(const UartConfig * uartConfig)
    : uartConfig_(uartConfig)
  {}

  size_t availableForRead() const;
  size_t availableForWrite() const;

  // assuming 'sizeof(buffer_) > 0'
  void on_loop_readLine_reset();

  // returns '(eol_ || overflow_)'
  bool on_loop_readLine();

  void on_loop(uint32_t now_ms) override;
};
