#include <Arduino.h>

#include "driver/uart.h"
#include "esp_err.h" // ESP_ERROR_CHECK()
#include "esp_log.h" // ESP_LOG*()

//#define COLLECT_MIN_MAX

#define CN_UART_NUM             0
#define CN_UART_BAUD            115200

#define PS_UART_NUM             1
#define PS_UART_BAUD            9600
#define PS_UART_RX_GPIO         16
#define PS_UART_TX_GPIO         17

#define EL_UART_NUM             2
#define EL_UART_BAUD            115200
#define EL_UART_RX_GPIO         26
#define EL_UART_TX_GPIO         25

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

static void log(const char *file, unsigned line, const char * function, const char * format, ...);
#define LOG(format, args...) log(__FILE__, __LINE__, __FUNCTION__, format, ##args)

static const char device_tag[] = "esp32";

static const char CMD_IDENTIFY[] = "*IDN?";

struct Runnable
{
  virtual void on_loop(uint32_t now_ms)
  {
    LOG("");
  }
};

struct Profile {
};

struct Controller
  : Runnable
{

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

  void on_ps_timeout()
  {
    LOG("");
  }

  void on_el_timeout()
  {
    LOG("");
  }

  void on_loop(uint32_t now_ms) override
  {
    //LOG("");
  }
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
        if ('\n' == b) {
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

#define cnLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct CN_Device
  : UartDevice
{
  CN_Device(
      const UartConfig * uartConfig)
    : UartDevice(uartConfig)
  {}

  void on_loop(uint32_t now_ms) override
  {
    // FIXME: implement necessary 'business logic'
    UartDevice::on_loop(now_ms);
  }
};

#define PxLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct PxDevice
  : UartDevice
{
  const char * const expected_id;
  uint32_t timeout_time_ms = 0;

  PxDevice(
      const UartConfig * uartConfig,
      const char * const expected_id_)
    : UartDevice(uartConfig)
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
    sync_discard_input,

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

  void write_cmd(const char * s) const {
    uartConfig_->write(s);
    uartConfig_->write('\n');
  }

  void command_try(
    const char * const command,
    unsigned answer_lines_count)
  {
    write_cmd(command);
    while (answer_lines_count--) {
      const uint32_t timeout_ms = 100;
      if (!uartConfig_->readLine_z(buffer_, sizeof(buffer_), timeout_ms)) {
        PxLOG("'%s' -> '%s'", command, "timeout");
      } else {
        PxLOG("'%s' -> '%s'", command, buffer_);
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
    on_loop_readLine_reset();

    // FIXME: remove the following from 'production' build.
#if 0
    // Note: the following is to simulate incomplete command sent before reboot
    uartConfig_->write(CMD_IDENTIFY);
#endif

    uartConfig_->write('\n'); // ends unfinished 'command' output

    constexpr uint32_t discard_input_timeout_ms = 100;
    timeout_time_ms = now_ms + discard_input_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  void on_state_sync_discard_input(uint32_t now_ms)
  {
    //PxLOG("");
    if (on_loop_readLine()) {
      PxLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_readLine_reset();
    }
    if (timeout_time_ms <= now_ms) {
      PxLOG("%s %s", "sync/discard_input", "timeout/success");
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_identify(uint32_t now_ms)
  {
    PxLOG("");
    on_loop_readLine_reset();

    write_cmd(CMD_IDENTIFY);

    constexpr uint32_t identify_timeout_ms = 120;
    timeout_time_ms = now_ms + identify_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  // assuming 'sizeof(buffer) > 0'
  void on_state_identify_read_answer(uint32_t now_ms)
  {
    //PxLOG("");
    if (on_loop_readLine()) {
      PxLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      if (eol_) {

        // TODO: check id match

        state = State(unsigned(state)+1); // ++state;
      } else if (overflow_) {

        // TODO: Mark device as unusable.
        // TODO: Retry identification after some delay.

        state = State(unsigned(state)+1); // ++state;
      } else if (timeout_time_ms <= now_ms) {
        PxLOG("%s %s id='%s'", "identify", "timeout", buffer_);

        // TODO: Mark device as unusable.
        // TODO: Retry identification after some delay.

        state = State(unsigned(state)+1); // ++state;
      }
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

  void on_loop(uint32_t now_ms) override
  {
    switch (state) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
      CASE_CALL(init)
      CASE_CALL(sync)
      CASE_CALL(sync_discard_input)
      CASE_CALL(identify)
      CASE_CALL(identify_read_answer)
      CASE_CALL(idle)
      CASE_CALL(active)
#undef CASE_CALL
      default:
        PxLOG("");
        break;
    }
  }
};

struct PS_Device
  : PxDevice
{
  PS_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}
};

struct EL_Device
  : PxDevice
{
  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}
};

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

static const UartConfig el_uartConfig = {
  .tag = "EL-u" MAKE_STRING(EL_UART_NUM),
  .hs = &Serial2,
  .baud = EL_UART_BAUD,
  .n = 2,
  .rxd_gpio = EL_UART_RX_GPIO,
  .txd_gpio = EL_UART_TX_GPIO,
};

#if 1
static const UartConfig * const xx_uartConfigs[] = {
  &cn_uartConfig,
  &ps_uartConfig,
  &el_uartConfig,
};

static void xx_uartConfigs_init() {
  for (unsigned i = 0; i < ARRAY_SIZE(xx_uartConfigs); ++i) {
    xx_uartConfigs[i]->init();
  }
}
#endif

static CN_Device cnDevice = {
  /*.uartConfig =*/ &cn_uartConfig,
};

// 'TENMA 72-2710 V6.6 SN:31428462'

static PS_Device psDevice {
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

static EL_Device elDevice = {
  /*.uartConfig =*/ &el_uartConfig,
  /*.expected_id =*/
  //"TENMA 72-13210 V2.50 SN:00012161",
  "TENMA 72-13210 V2.20 SN:00010724",
};

static const char * const elDevice_commands[] = {
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
static void elDevice_commands_try()
{
  const char * CMD_DEVINFO = ":SYSTem:DEVINFO?";
  elDevice.command_try(CMD_DEVINFO, 7);
    ARRAY_SIZE(elDevice_commands);
#if 0
  elDevice.commands_try(
    elDevice_commands,
    ARRAY_SIZE(elDevice_commands));
#endif
}
static Runnable * const uartDevices[] = {
  &cnDevice,
  &psDevice,
  &elDevice,
};

static void uartDevices_on_loop(int32_t now_ms) {
  for (unsigned i = 0; i < ARRAY_SIZE(uartDevices); ++i) {
    uartDevices[i]->on_loop(now_ms);
  }
}

Controller controller;

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

static void report_config()
{
  Serial.print("device: '"); Serial.print(device_tag); Serial.println("'");
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
  const auto now_s = now_ms / 1000;

#if 1
  static uint32_t report_config_s = 0;
  if (now_s - report_config_s >= 5*60) {
    report_config();
    report_config_s = now_s;
  }
#endif

  static uint32_t report_count_s = 0;
  if (now_s - report_count_s >= 5) {
    Serial.print(device_tag);
    Serial.print(" now_ms="); Serial.print(now_ms);
    Serial.print(" loop_count="); Serial.println(loop_count);

    report_count_s = now_s;
  }

  //psDevice.identify();
  //elDevice.identify();

  //cnDevice.poll();
  //psDevice.poll();
  //elDevice.poll();

  uartDevices_on_loop(now_ms);
  controller.on_loop(now_ms);

#if 0
  if (psDevice.state == PxDevice::State::idle
  &&  elDevice.state == PxDevice::State::idle) {
    LOG_FUNCTION
    delay(15000);
  }
#endif

  ++loop_count;
}
