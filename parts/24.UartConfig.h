
#define ucLOG(format, args...) LOG("%s| " format, tag, ##args)
struct UartConfig {
  const char * const tag;
  HardwareSerial * hs;
  uint32_t baud;

  uint8_t n;

  int8_t rxd_gpio;
  int8_t txd_gpio;

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
  */
  void init() const {
    hs->begin(baud, SERIAL_8N1, rxd_gpio, txd_gpio);
    ucLOG(". Rx GPIO=%u", rxd_gpio);
    ucLOG(". Tx GPIO=%u", txd_gpio);
    ucLOG(". baud=%u", baud);
  }

  void timeout(uint32_t timeout_ms) const {
    hs->setTimeout(timeout_ms);
  }

  size_t read(char * p, size_t size) const {
    return hs->readBytes(p, size);
  }

  size_t read(byte * p, size_t size, uint32_t timeout_ms) const {
    timeout(timeout_ms);
    return read( (char*)p, size);
  }

  size_t readLine(byte * p, size_t size, uint32_t timeout_ms) const {
    timeout(timeout_ms);
    return hs->readBytesUntil('\n', p, size);
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

  size_t availableForRead() const {
    return hs->available();
  }

  size_t availableForWrite() const {
    return hs->availableForWrite();
  }
};
