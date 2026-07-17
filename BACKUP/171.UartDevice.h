
#define udLOG(format, args...) LOG("%s| " format, uartConfig->tag, ##args)
struct UartDevice
  : Runnable
{
  const UartConfig * uartConfig;
  size_t i = 0;
  static constexpr size_t buffer_size = 64;
  byte buffer[buffer_size];

  UartDevice(const UartConfig * uartConfig_)
    : uartConfig(uartConfig_)
  {}

/*
  void init() const {
    uartConfig->init();
  }
*/

  size_t availableForRead() const {
    return uartConfig->availableForRead();
  }

  size_t read(byte * p, size_t size, uint32_t timeout_ms) const {
    return uartConfig->read(p, size, timeout_ms);
  }

  size_t read(uint32_t timeout_ms) {
    return read(buffer, sizeof(buffer), timeout_ms);
  }

  byte read_byte(uint32_t timeout_ms) const {
    byte b = 0;
    read(&b, sizeof(b), timeout_ms);
    return b;
  }

  size_t readLine(byte * p, size_t size, uint32_t timeout_ms) const {
    return uartConfig->readLine(p, size, timeout_ms);
  }

  size_t readLine(uint32_t timeout_ms) {
    size_t len = readLine(buffer, sizeof(buffer) - 1, timeout_ms);
    buffer[len] = '\0';
    return len;
  }

  size_t availableForWrite() const {
    return uartConfig->availableForWrite();
  }

  void write(const byte b) const {
    uartConfig->write(b);
  }

  void write(const byte * p, size_t size) const {
    uartConfig->write(p, size);
  }

  void write(const char * s) const {
    uartConfig->write(s);
  }

  // wait for all output data to be placed into output FIFO
  void finish_output() const {
    uartConfig->finish_output();
  }

  void discard_input(uint32_t timeout_ms) {
#if defined COLLECT_MIN_MAX
    const auto ms0 = millis();
#endif
    readLine(timeout_ms);

#if defined COLLECT_MIN_MAX
    const auto discard_input_ms = millis() - ms0;

    static uint32_t discard_input_ms_min = 1000;
    if (discard_input_ms_min > discard_input_ms) {
      udLOG(". discard_input_ms_min=%u -> %u", discard_input_ms_min, discard_input_ms);
      discard_input_ms_min = discard_input_ms;
    }

    static uint32_t discard_input_ms_max = 0;
    if (discard_input_ms_max < discard_input_ms) {
      udLOG(". discard_input_ms_max=%u -> %u", discard_input_ms_max, discard_input_ms);
      discard_input_ms_max = discard_input_ms;
    }
#endif
  }


  void onBufferOverflow() {
    udLOG("buffer full without CR or LF (sizeof(buffer)=%zu)", sizeof(buffer));
    udLOG("buffer='%s'", buffer);
    //i = 0;
  }

  void onLine() {
    udLOG("buffer='%s'", buffer);
    //i = 0;
  }

  // assuming 'sizeof(buffer) > 0'
  // assuming 'i < sizeof(buffer)'
  void on_availableForRead(size_t size) {
    while (size--) {
      const auto b = read_byte(0);
      if ('\n' == b || '\r' == b) { // EOL = End Of Line
        buffer[i] = '\0';
        onLine();
        i = 0; // start next logical read loop from the beginning
      } else {
        if (i < (sizeof(buffer) - 1)) {
          buffer[i++] = b;
        } else {
          // Note: last received symbol is discarded here
          buffer[i] = '\0';
          onBufferOverflow();
          i = 0; // start next logical read loop from the beginning
        }
      }
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    on_availableForRead(availableForRead());
  }
};
