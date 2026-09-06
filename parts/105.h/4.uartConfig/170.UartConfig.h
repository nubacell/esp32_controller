
#define ucLOG(format, args...) LOG("%s| " format, tag, ##args)
struct UartConfig {
  const char * const tag;
  HardwareSerial * hs;
  uint32_t baud;

  uint8_t n;

  int8_t rxd_gpio;
  int8_t txd_gpio;

  size_t rxBufferSize;
  size_t txBufferSize;

  /*
    ESP32 uses 'HardwareSerial' to represent Arduino's 'Serial*'.

    class HardwareSerial {
      void begin(
        unsigned long baud,
        uint32_t config = SERIAL_8N1,
        int8_t rxPin = -1,
        int8_t txPin = -1,
        bool invert = false,
        unsigned long timeout_ms = 20000UL,
        uint8_t rxfifo_full_thrhd = 120
      );
    };
    uart_t *uartBegin(
        uint8_t uart_nr,
        uint32_t baudrate,
        uint32_t config,
        int8_t rxPin,
        int8_t txPin,
        uint32_t rx_buffer_size,
        uint32_t tx_buffer
        uint8_t rxfifo_full_thrhd)

    Note: 'setRxBufferSize()' and 'setTxBufferSize()' must be invoked before 'begin()'
    size_t HardwareSerial::setRxBufferSize(size_t new_size);
    size_t HardwareSerial::setTxBufferSize(size_t new_size);
  */
  void init() const {
    if (rxBufferSize) { hs->setRxBufferSize(rxBufferSize); }
    if (txBufferSize) { hs->setTxBufferSize(txBufferSize); }
    hs->begin(baud, SERIAL_8N1, rxd_gpio, txd_gpio);
    // FIXME: Move the following to something like uartConfig_report_config()':
    ucLOG(". Rx GPIO=%i", rxd_gpio);
    ucLOG(". Tx GPIO=%i", txd_gpio);
    ucLOG(". baud=%u", baud);
    if (rxBufferSize) { ucLOG(". rxBufferSize=%u", rxBufferSize); }
    if (txBufferSize) { ucLOG(". txBufferSize=%u", txBufferSize); }
    //hs->setRxFIFOFull(60);
  }

  uint32_t baudRate() const {
    return hs->baudRate();
  }

  void baudRate(uint32_t baud) const {
    hs->updateBaudRate(baud);
  }

  size_t availableForRead() const {
    return hs->available();
  }

  void timeout_(uint32_t timeout_ms) const {
    hs->setTimeout(timeout_ms);
  }

  size_t read_(byte * p, size_t size) const {
    return hs->readBytes( (char*)p, size);
  }

  size_t read(byte * p, size_t size, uint32_t timeout_ms) const {
    timeout_(timeout_ms);
    return read_(p, size);
  }

  byte read_byte(uint32_t timeout_ms) const {
    byte b = 0;
    read(&b, sizeof(b), timeout_ms);
    return b;
  }

  size_t readLine(byte * p, size_t size, uint32_t timeout_ms) const {
    timeout_(timeout_ms);
    return hs->readBytesUntil('\n', p, size);
  }

  size_t readLine_z(byte * p, size_t size, uint32_t timeout_ms) const {
    if (!size) {
      return 0;
    } else {
      size_t len = readLine(p, size - 1, timeout_ms);
      p[len] = '\0';
      return len;
    }
  }

  size_t availableForWrite() const {
    return hs->availableForWrite();
  }

  void write(const byte b) const {
    hs->write(b);
  }

  void write(const byte * p, size_t size) const {
    hs->write(p, size);
  }

  void write(const char * s) const {
    hs->write(s);
  }

  // wait for all output data to be placed into output FIFO
  void finish_output() const {
    hs->flush();
  }
};
