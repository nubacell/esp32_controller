
struct UartMiniTask
  : MiniTask
{
  UartDevice * uartDevice_;

  UartMiniTask(
      MiniTaskObserver * miniTaskObserver,
      UartDevice * uartDevice)
    : MiniTask(miniTaskObserver)
    , uartDevice_(uartDevice)
  {}

  size_t availableForRead() const {
    return uartDevice_->availableForRead();
  }

  size_t read(byte * p, size_t size, uint32_t timeout_ms) const {
    return uartDevice_->read(p, size, timeout_ms);
  }

  byte read_byte(uint32_t timeout_ms) const {
    return uartDevice_->read_byte(timeout_ms);
  }

  void write(const byte b) const {
    uartDevice_->write(b);
  }

  void write(const byte * p, size_t size) const {
    uartDevice_->write(p, size);
  }

  void write(const char * s) const {
    uartDevice_->write(s);
  }

  void write_cmd(const char * s) const {
    write(s);
    write('\n');
  }
};
