#include <Arduino.h>

#include "driver/uart.h"
#include "esp_err.h" // ESP_ERROR_CHECK()
#include "esp_log.h" // ESP_LOG*()

//#define COLLECT_MIN_MAX

#define LOG_FUNCTION \
    Serial.print("--- "); \
    Serial.print(__func__); \
    Serial.println(" ---");

#define LOG_STARTED \
    Serial.print("--- "); \
    Serial.print(__func__); \
    Serial.print(" started"); \
    Serial.println(" ---");

#define LOG_FINISHED \
    Serial.print("--- "); \
    Serial.print(__func__); \
    Serial.print(" finished"); \
    Serial.println(" ---");

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#define MAKE_STRING_HELPER2(s) #s
#define MAKE_STRING_HELPER1(s) MAKE_STRING_HELPER2(s)
#define MAKE_STRING(s) MAKE_STRING_HELPER1(s)

#define CN_UART_NUM             0
#define CN_UART_BAUD            115200

#define PS_UART_NUM             1
#define PS_UART_BAUD            9600
#define PS_UART_RX_GPIO         16
#define PS_UART_TX_GPIO         17

#define PL_UART_NUM             2
#define PL_UART_BAUD            115200
#define PL_UART_RX_GPIO         26
#define PL_UART_TX_GPIO         25

static const char CMD_IDENTIFY[] = "*IDN?";

static void log(const char *file, unsigned line, const char * function, const char * format, ...);
#define LOG(format, args...) log(__FILE__, __LINE__, __FUNCTION__, format, ##args)

struct Profile {
};

struct Controller {

  enum class State {

    /*
      Check if PS and PL are present and usable.
      Make sure they are turned off.
    */
    init,

    /*
      Waiting for commands from raspberry on uart #0
    */
    idle,

    /*
      test running
    */
    active,
  };
  State state = State::init;

};

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

#define udLOG(format, args...) LOG("%s| " format, uartConfig->tag, ##args)
struct UartDevice {
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

  size_t read(byte * p, size_t size, uint32_t timeout_ms) const {
    return uartConfig->read(p, size, timeout_ms);
  }

  size_t read(uint32_t timeout_ms) {
    return read(buffer, sizeof(buffer), timeout_ms);
  }

  size_t readLine(byte * p, size_t size, uint32_t timeout_ms) const {
    return uartConfig->readLine(p, size, timeout_ms);
  }

  size_t readLine(uint32_t timeout_ms) {
    size_t len = readLine(buffer, sizeof(buffer) - 1, timeout_ms);
    buffer[len] = '\0';
    return len;
  }

  void write(const byte b) {
    uartConfig->write(b);
  }

  void write(const byte * p, size_t size) {
    uartConfig->write(p, size);
  }

  void write(const char * s) const {
    uartConfig->write(s);
  }

  // wait for all output data to be placed into output FIFO
  void finish_output() const {
    uartConfig->finish_output();
  }

  size_t availableForRead() const {
    return uartConfig->availableForRead();
  }

  size_t availableForWrite() const {
    return uartConfig->availableForWrite();
  }

  void empty_input(uint32_t timeout_ms) {
#if defined COLLECT_MIN_MAX
    const auto ms0 = millis();
#endif
    readLine(timeout_ms);

#if defined COLLECT_MIN_MAX
    const auto empty_input_ms = millis() - ms0;

    static uint32_t empty_input_ms_min = 1000;
    if (empty_input_ms_min > empty_input_ms) {
      udLOG(". empty_input_ms_min=%u -> %u", empty_input_ms_min, empty_input_ms);
      empty_input_ms_min = empty_input_ms;
    }

    static uint32_t empty_input_ms_max = 0;
    if (empty_input_ms_max < empty_input_ms) {
      udLOG(". empty_input_ms_max=%u -> %u", empty_input_ms_max, empty_input_ms);
      empty_input_ms_max = empty_input_ms;
    }
#endif
  }


  byte read_byte(uint32_t timeout_ms) const {
    byte b = 0;
    read(&b, sizeof(b), timeout_ms);
    return b;
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

/*
  void poll() {
    on_availableForRead(availableForRead());
  }
*/

  virtual
  void on_loop(uint32_t now_ms)
  {
    on_availableForRead(availableForRead());
  }
};

#define PxLOG udLOG
struct PxDevice
  : UartDevice
{
  const char * const expected_id;
  uint32_t timeout_ms = 0;
  bool identify_bufferOverflow = false;

  PxDevice(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : UartDevice(uartConfig_)
    , expected_id(expected_id_)
  {}

  enum class State {
    /*
      Check if Px (PS or PL) device is present and usable.
      Make sure Px device is turned off.

        * action[s]
        * waiting[s]
        * timeout[s]
        * on timeout
        * on explicit failure
        * on explicit success

      ...
    */
    init,

    /*
      Sync UART communication state to fix possible incomplete send/receive
      exchange left due to reboot in the middle of unfinished communication
      send/receive exchange.

        - Send '\n' to finish possible incomplete command.
        - Wait to receive possible answer
        - Timeout shall be big enough for remote side to receive command,
          process it and transfer respective answer. Minimum can be computed by
          multiplication of baud rate by length of longest possible answer. In
          practice expected timeout shall be verified by collecting statistics.
        - Timeout is success. Failure is impossible.
        - Success prior to timeout is possible if received message matches
          template of longest possible answer.
    */
    sync,
    sync_empty_input,

    /*
      Identify attached Px (PS or PL) device.

        - Send IDENTIFY command ('*IDN?').
        - Receive identifycation string.
        - Timeout shall be big enough for remote side to receive command,
          process it and transfer respective answer. Minimum can be computed by
          multiplication of baud rate by length of longest possible answer. In
          practice expected timeout shall be verified by collecting statistics.
        - Timeout is failure.
        - Success is if received answer matches expected template. In simplest
          case answer shall be just not empty.
    */
    identify,
    identify_read_answer,

    /*
      Waiting for commands from controller.
      Periodically check if Px device is present.
    */
    idle,

    /*
      test running
    */
    active,
  };
  State state = State::init;

  void readLine_v(uint32_t timeout_ms) {
    if (!readLine(timeout_ms)) {
      PxLOG("!!! nothing received");
#if 0
    } else {
      PxLOG("buffer='%s'", buffer);
#endif
    }
  }

  void write_cmd(const char * s) {
    write(s);
    write('\n');
  }

  void identify() {
#if defined COLLECT_MIN_MAX
    const auto ms0 = millis();
#endif

    write_cmd(CMD_IDENTIFY);
    //const auto write_ms = millis();

    finish_output();
    //const auto flush_ms = millis();

    constexpr uint32_t identify_timeout_ms = 120;
    readLine_v(identify_timeout_ms);

#if defined COLLECT_MIN_MAX
    const auto poll_ms = millis();

    //PxLOG("flush_ms=%u poll_ms=%u", flush_ms - ms0, poll_ms - flush_ms);

    const auto identify_ms = poll_ms - ms0;
    static uint32_t identify_ms_min = 1000;
    if (identify_ms_min > identify_ms) {
      PxLOG(". identify_ms_min=%u -> %u", identify_ms_min, identify_ms);
      identify_ms_min = identify_ms;
    }

    static uint32_t identify_ms_max = 0;
    if (identify_ms_max < identify_ms) {
      PxLOG(". identify_ms_max=%u -> %u", identify_ms_max, identify_ms);
      identify_ms_max = identify_ms;
    }
#endif

    if (strcmp(expected_id, (const char*) buffer)) {
      PxLOG("identify mismatch");
      PxLOG(". expected '%s'", expected_id);
      PxLOG(". actual   '%s'", buffer);
    }
  }

  void sync()
  {
// FIXME: remove the following from 'production' build.
#if 0
    // Note: the following is to simulate incomplete command sent before reboot
    write(CMD_IDENTIFY);
#endif

    write('\n'); // ends unfinished 'command' output
    finish_output();
    constexpr uint32_t empty_input_timeout_ms = 100;
    empty_input(empty_input_timeout_ms);
  }

/*
  void init() {
    UartDevice::init();
    sync();
    identify();
    PxLOG("buffer='%s'", buffer);
  }
*/

  void command_try(
    const char * const command,
    unsigned answer_lines_count)
  {
    write_cmd(command);
    while (answer_lines_count--) {
      const uint32_t timeout_ms = 100;
      if (!readLine(timeout_ms)) {
        PxLOG("'%s' -> '%s'", command, "timeout");
      } else {
        PxLOG("'%s' -> '%s'", command, buffer);
      }
    }
  }

  void commands_try(
    const char * const * const commands,
    unsigned commands_count)
  {
    for (unsigned i = 0; i < commands_count; ++i) {
      const char * const command = commands[i];
      command_try(command, 1);
    }
  }

  void on_state_init(uint32_t now_ms)
  {
    PxLOG("");
    state = State(unsigned(state)+1); // ++state;
  }

  void on_state_sync(uint32_t now_ms)
  {
    PxLOG("");

    // FIXME: remove the following from 'production' build.
#if 0
    // Note: the following is to simulate incomplete command sent before reboot
    write(CMD_IDENTIFY);
#endif

    write('\n'); // ends unfinished 'command' output

    constexpr uint32_t empty_input_timeout_ms = 100;
    timeout_ms = now_ms + empty_input_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  void on_state_sync_empty_input(uint32_t now_ms)
  {
    //PxLOG("");
    empty_input(0);
    if (timeout_ms <= now_ms) {
      PxLOG("%s %s", "sync/empty_input", "timeout/success");
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_identify(uint32_t now_ms)
  {
    PxLOG("");
    i = 0; // store answer starting from beginning of buffer
    identify_bufferOverflow = false;

    write_cmd(CMD_IDENTIFY);

    constexpr uint32_t identify_timeout_ms = 120;
    timeout_ms = now_ms + identify_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  // assuming 'sizeof(buffer) > 0'
  // assuming 'i < sizeof(buffer)'
  void on_state_identify_read_answer(uint32_t now_ms)
  {
    //PxLOG("");
    size_t availableForRead_ = availableForRead();
    while (availableForRead_--) {
      const auto b = read_byte(0);
      if ('\n' == b || '\r' == b) { // EOL = End Of Line
        buffer[i] = '\0';
        // Note: 'i' holds length of answer
        // TODO: make copy of 'buffer' for later use
        PxLOG("%s %s id='%s'", "identify", "success", buffer);

        state = State(unsigned(state)+1); // ++state;
        return;
      } else {
        if (i < (sizeof(buffer) - 1)) {
          buffer[i++] = b;
        } else {
          // Note: last received symbol is discarded here
          buffer[i] = '\0';
          identify_bufferOverflow = true;
        }
      }
    }
    if (timeout_ms <= now_ms) {
      PxLOG("%s %s id='%s'", "identify", "timeout", buffer);
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_idle(uint32_t now_ms)
  {
    //PxLOG("");
  }

  void on_state_active(uint32_t now_ms)
  {
    PxLOG("");
  }

  //virtual
  void on_loop(uint32_t now_ms) override
  {
    switch (state) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
      CASE_CALL(init)
      CASE_CALL(sync)
      CASE_CALL(sync_empty_input)
      CASE_CALL(identify)
      CASE_CALL(identify_read_answer)
      CASE_CALL(idle)
      CASE_CALL(active)
#undef CASE_CALL
      default:
        UartDevice::on_loop(now_ms);
        break;
    }
  }
};

Controller controller;

static const UartConfig cn_uartConfig = {
  .tag =  "CN-u" MAKE_STRING(CN_UART_NUM),
  .hs = &Serial,
  .baud = CN_UART_BAUD,
  .n = 0,
  .rxd_gpio = -1, // '-1' == 'do not change'
  .txd_gpio = -1, // '-1' == 'do not change'
};

static const UartConfig ps_uartConfig = {
  .tag =  "PS-u" MAKE_STRING(PS_UART_NUM),
  .hs = &Serial1,
  .baud = PS_UART_BAUD,
  .n = 1,
  .rxd_gpio = PS_UART_RX_GPIO,
  .txd_gpio = PS_UART_TX_GPIO,
};

static const UartConfig pl_uartConfig = {
  .tag = "PL-u" MAKE_STRING(PL_UART_NUM),
  .hs = &Serial2,
  .baud = PL_UART_BAUD,
  .n = 2,
  .rxd_gpio = PL_UART_RX_GPIO,
  .txd_gpio = PL_UART_TX_GPIO,
};

#if 1
static const UartConfig * const xx_uartConfigs[] = {
  &cn_uartConfig,
  &ps_uartConfig,
  &pl_uartConfig,
};

static void xx_uartConfigs_init() {
  for (unsigned i = 0; i < ARRAY_SIZE(xx_uartConfigs); ++i) {
    xx_uartConfigs[i]->init();
  }
}
#endif

static UartDevice cnDevice = {
  /*.uartConfig =*/ &cn_uartConfig,
};

// 'TENMA 72-2710 V6.6 SN:31428462'

static PxDevice psDevice {
  /*.uartConfig =*/ &ps_uartConfig,
  /*.expected_id =*/ "TENMA 72-2710 V6.6 SN:31428462",
};

// "TENMA 72-2710 V6.6 SN:31428462"
/*
  settings:

    ISET<X>:<NR2>
    VSET<X>:<NR2>
    ISET<X>?
    VSET<X>?

  actual output

    IOUT<X>?
    VOUT<X>?

  output on/off

    OUT<Boolean>

  STATUS?
*/
static const char * const psDevice_commands[] = {
  "ISET1?",
  "VSET1?",
  "IOUT1?",
  "VOUT1?",
  "STATUS?",
};
static void psDevice_commands_try()
{
  psDevice.commands_try(
    psDevice_commands,
    ARRAY_SIZE(psDevice_commands));
}

// 'TENMA 72-13210 V2.20 SN:00010724'
// 'TENMA 72-13210 V2.50 SN:00012161'

static PxDevice plDevice = {
  /*.uartConfig =*/ &pl_uartConfig,
  /*.expected_id =*/
  //"TENMA 72-13210 V2.50 SN:00012161",
  "TENMA 72-13210 V2.20 SN:00010724",
};

static const char * const plDevice_commands[] = {
  ":SYSTem:BAUD?",
  ":SYSTem:BEEP?",
  ":SYSTem:PORT?",
  //":SYSTem:DEVINFO?", // returns 7 lines
  ":STATus?",
  ":INPut?",
  ":VOLTage?",
  ":VOLTage:UPPer?",
  ":VOLTage:LOWer?",
  ":CURRent?",
  ":CURRent:UPPer?",
  ":CURRent:LOWer?",
  ":MEASure:CURRent?",
  ":MEASure:VOLTage?",
  ":MEASure:POWer?",
};
static void plDevice_commands_try()
{
  const char * CMD_DEVINFO = ":SYSTem:DEVINFO?";
  plDevice.command_try(CMD_DEVINFO, 7);
    ARRAY_SIZE(plDevice_commands);
#if 0
  plDevice.commands_try(
    plDevice_commands,
    ARRAY_SIZE(plDevice_commands));
#endif
}
static UartDevice * const uartDevices[] = {
  &cnDevice,
  &psDevice,
  &plDevice,
};

static void uartDevices_on_loop(int32_t now_ms) {
  for (unsigned i = 0; i < ARRAY_SIZE(uartDevices); ++i) {
    uartDevices[i]->on_loop(now_ms);
  }
}

void hex_dump(const uint8_t * data, size_t size)
{
  unsigned i;
  for (i = 0; i < size; ++i) {
    if (0 == (i % 16)) {
      if (i) {
        Serial.println("");
      }
      Serial.print(". . ");
      Serial.print(i/16, HEX);
      Serial.print(":");
    } else if (0 == (i % 4)) {
      Serial.print(" ");
    }
    Serial.print(" ");
    const uint8_t b = data[i];
    Serial.print((b >> 4) & 0x0F, HEX);
    Serial.print( b       & 0x0F, HEX);
  }
  if (i) {
    Serial.println("");
  }
}

static void log_write(unsigned line, const char * function, const char * s)
{
  const auto ms = millis();
  Serial.print(ms);
  Serial.print(" ");
  Serial.print(function);
  Serial.print(":");
  Serial.print(line);
  Serial.print("| ");

  Serial.println(s);
}

void log(const char *file, unsigned line, const char * function, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);

  constexpr size_t buffer_size = 100;
  char buffer[buffer_size];
  size_t output_length = vsnprintf(buffer, sizeof(buffer), format, argsList);
  log_write(line, function, buffer);

  va_end(argsList);

  size_t needed_size = output_length + 1; // add space for terminating '\0'
  if (buffer_size < needed_size) {
    snprintf(buffer, sizeof(buffer),
        "ALERT: logger buffer overflow (needed=%zu available=%zu)",
        needed_size, sizeof(buffer));
    log_write(__LINE__, __FUNCTION__, buffer);
  }
}

#if 0
void serial_setup()
{
  //Serial.begin(115200);
  xx_uartConfigs_init();
  LOG_FUNCTION
  LOG("uxTaskPriorityGet()=%i", uxTaskPriorityGet(NULL));

  LOG_FUNCTION

#if 0
  Serial.println(ps_uart_cfg.tag);
  xx_uart_cfg_hw_dump(&ps_uart_cfg);
  Serial.println(pl_uart_cfg.tag);
  xx_uart_cfg_hw_dump(&pl_uart_cfg);
  LOG_FUNCTION
#endif
}
#endif

void setup()
{
  //serial_setup();
  xx_uartConfigs_init();

  //cnDevice.init();
  //psDevice.init();
  //plDevice.init();

  //psDevice_commands_try();
  //plDevice_commands_try();
}

void loop()
{
  //LOG_FUNCTION

  static uint32_t loop_count = 0;

  uint32_t now_ms = millis();

  //psDevice.identify();
  //plDevice.identify();

  //cnDevice.poll();
  //psDevice.poll();
  //plDevice.poll();

  uartDevices_on_loop(now_ms);

  if (psDevice.state == PxDevice::State::idle
  &&  plDevice.state == PxDevice::State::idle) {
    LOG_FUNCTION
    delay(15000);
  }

  ++loop_count;
}
