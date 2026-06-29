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
//#define EL_UART_BAUD            115200
#define EL_UART_BAUD            38400
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

#define CONSTSTRLEN(s) (sizeof(s)-1)

#define MAKE_STRING_HELPER2(s) #s
#define MAKE_STRING_HELPER1(s) MAKE_STRING_HELPER2(s)
#define MAKE_STRING(s) MAKE_STRING_HELPER1(s)

static void log(const char *file, unsigned line, const char * function, const char * format, ...);
#define LOG(format, args...) log(__FILE__, __LINE__, __FUNCTION__, format, ##args)

typedef void (*command_fn_t)(const char * args);

typedef struct commands_map_s {
  const char * command_s;
  size_t command_len;
  command_fn_t command_fn;
} commands_map_t;

//extern
//const commands_map_t commands_map[];

void print_commands();

command_fn_t
command_s_to_fn(
    const char * command_s,
    size_t command_len);

// TODO: Use 'enum class' instead of 'unsigned'

extern const char * const el_functions[];
const char * el_function_to_string(unsigned function);

extern const char * const el_ranges[];
const char * el_range_to_string(unsigned cc_range);

static const char device_tag[] = "esp32";

static const char CMD_IDENTIFY[] = "*IDN?";

struct Runnable
{
  virtual void on_loop(uint32_t now_ms)
  {
    LOG("");
  }
};

const char * skip_whitespace(const char * p)
{
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  while (' ' == *p || '\t' == *p) {
    ++p;
    //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  }
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  return p;
}

enum class PSState {
  off,
  CC,
  CV,
};
const char * to_string(PSState psstate);

struct PSConfig {
  PSState state_;
  double i_;
  double v_;

  void dump() const {
    LOG(". . %s state='%s' I=%.3f V=%.3f", "PS", to_string(state_), i_, v_);
  }
};

// Note: We age going to use "EL" in "CC" mode only

enum class ELState {
  off,
  CC,
  CV,
};
const char * to_string(ELState elstate);

struct ELConfig {
  ELState state_;
  uint8_t range_;
  double level_;

  void dump() const {
    LOG(". . %s state_=%u/'%s' range=%u/'%s' level=%.6f", "EL",
        state_, to_string(state_), range_, el_range_to_string(range_), level_);
  }

  void apply(const char * args)
  {
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      state_ = (ELState) strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      range_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      level_ = strtod(args, &endptr);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
  }
};

struct ProfileStep {
  uint32_t substeps_count_ = 0;
  uint32_t measure_period_ms_;
  //PSConfig psconfig_;
  ELConfig elconfig_;

  void dump() const {
    LOG(". this=%p substeps_count=%u measure_period_ms=%u",
        this, substeps_count_, measure_period_ms_);
    //psconfig_.dump();
    elconfig_.dump();
  }

  void apply(const char * args)
  {
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      substeps_count_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      measure_period_ms_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    elconfig_.apply(args);
  }
};

constexpr uint32_t profile_steps_count_max = 100;
static ProfileStep profileSteps_[profile_steps_count_max];

struct Profile {
  uint32_t steps_count_ = 0;

  void dump() const {
    LOG("Profile: steps_count=%u", steps_count_);
  }

  void dump_steps() const {
    for (uint32_t i = 0; i < steps_count_; ++i) {
      ProfileStep * step = &profileSteps_[i];
      step->dump();
    }
  }

  void steps_count(uint32_t new_steps_count) {
    if (new_steps_count > profile_steps_count_max) {
      LOG("new_steps_count=%u > profile_steps_count_max=%u", new_steps_count, profile_steps_count_max);
    } else {
      steps_count_ = new_steps_count;
    }
  }
};

Profile profile;

const char * to_string(PSState psstate) {
  switch (psstate) {
#define CASE_RETURN(x) case PSState::x: return #x;
    CASE_RETURN(off);
    CASE_RETURN(CC);
    CASE_RETURN(CV);
#undef CASE_RETURN
    default: return "?";
  }
}

const char * to_string(ELState elstate) {
  switch (elstate) {
#define CASE_RETURN(x) case ELState::x: return #x;
    CASE_RETURN(off);
    CASE_RETURN(CC);
    CASE_RETURN(CV);
#undef CASE_RETURN
    default: return "?";
  }
}

void profile_steps(const char * args)
{
  //LOG("args=%p", args);
  args = skip_whitespace(args);
  //LOG("args=%p", args);
  if (*args) {
    char * endptr = NULL;
    uint32_t new_steps_count = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    LOG("new_steps_count=%u", new_steps_count);
    profile.steps_count(new_steps_count);
  }
  LOG("profile_steps=%u", profile.steps_count_);
}

void profile_steps_dump(const char * args)
{
  (void) args;
  profile.dump_steps();
}

void profile_step(const char * args)
{
  args = skip_whitespace(args);
  if (!*args) {
    LOG("format: step substeps_count period_ms EL_state EL_range EL_level");
  } else {
    char * endptr = NULL;
    unsigned step = strtoul(args, &endptr, 0);

    if (step >= profile.steps_count_) {
      LOG("ERROR: step=%u >= profile.steps_count=%u", step, profile.steps_count_);
    } else {
      ProfileStep * profileStep = &profileSteps_[step];
      if (endptr && *endptr) {
        profileStep->apply(endptr);
      }
      profileStep->dump();
    }
  }
}

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
    ucLOG(". Rx GPIO=%i", rxd_gpio);
    ucLOG(". Tx GPIO=%i", txd_gpio);
    ucLOG(". baud=%u", baud);
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

#define cnLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct CN_Device
  : UartDevice
{
  CN_Device(
      const UartConfig * uartConfig)
    : UartDevice(uartConfig)
  {
    cnLOG("device_tag='%s'", device_tag);
  }

  void on_loop_on_line(const char * s)
  {
    const char * p = strchr(s, ' ');
    size_t len;
    const char * args;
    if (!p) {
      len = strlen(s);
      args = s + len;
    } else {
      len = p - s;
      args = p + 1;
    }
    cnLOG("len=%zu args='%s'", len, args);

    command_fn_t command_fn = command_s_to_fn(s, len);

    if (!command_fn) {
      printf("There is no command '%*s'\n", (int)len, s);
      print_commands();
    } else {
      command_fn(args);
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    if (on_loop_readLine()) {
      cnLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_on_line( (const char *) buffer_);
      on_loop_readLine_reset();
    }
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
    //uartConfig_->write("*CLS");
#if 0
    // Note: the following is to simulate incomplete command sent before reboot
    //write_cmd(CMD_IDENTIFY);
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
      }
    } else if (timeout_time_ms <= now_ms) {
      PxLOG("%s %s id='%s'", "identify", "timeout", buffer_);

      // TODO: Mark device as unusable.
      // TODO: Retry identification after some delay.

      //state = State::identify;
      //state = State::init;
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_idle(uint32_t now_ms)
  {
    //PxLOG("");
    if (on_loop_readLine()) {
      PxLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_readLine_reset();
    }
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

  // 31 2e 30 30 30 0a                                 |1.000.|
  // 31 2e 32 33 34 0a                                 |1.234.|
  // 35 2e 31 30 30 0a                                 |5.100.|         ISET1:12.345    ISET1:9.999
  void get_I()
  {
    write_cmd("ISET1?");
  }

  // 30 39 2e 30 30 0a                                 |09.00.|
  // 31 32 2e 33 34 0a                                 |12.34.|
  // 30 30 2e 31 32 0a                                 |00.12.|
  // 33 31 2e 30 30 0a                                 |31.00.|
  // 30 30 2e 30 30 0a                                 |00.00.|
  void get_V()
  {
    write_cmd("VSET1?");
  }

  void set_I(double v)
  {
    char cmd[sizeof("ISET1:9.999")];
    snprintf(cmd, sizeof(cmd), "ISET1:%.3f", v);
    write_cmd(cmd);

    get_I();
  }

  void set_V(double v)
  {
    char cmd[sizeof("VSET1:99.99")];
    snprintf(cmd, sizeof(cmd), "VSET1:%.2f", v);
    write_cmd(cmd);

    get_V();
  }
};

struct EL_Device
  : PxDevice
{
  uint8_t function_;
  uint8_t cc_range_;
  uint8_t cv_range_;

  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}

  void function() const
  {
    const char * cmd = "INPut:FUNCtion?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void function_xx(unsigned v)
  {
    char cmd[sizeof("INPut:FUNCtion XX")];
    snprintf(cmd, sizeof(cmd), "INPut:FUNCtion %s", el_function_to_string(v));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    function_ = 0;
  }

  void function_CC()
  {
    function_xx(0);
  }

  void function_CV()
  {
    function_xx(1);
  }

  /*
    Constant Current

    Example: Set the load to the constant current function, and the main current
    value to 50A, and the programming command is as follows:

    INPut:FUNCtion CC                       // Switch to the constant current test function
    STATic:CC:RANGe 0                       // Set the constant current range to the high range
    STATic:CC:HIGH:LEVel 50.0               // Set the load current to 50.0A
    STATic:CC:HIGH:SLEWRate:RAIse 600       // Set the rise slew to 600.0A/ms
    STATic:CC:HIGH:SLEWRate:FALL 600        // Set the fall slew to 600.0A/ms
    INPut:STATe ON                          // Turn on the load input
  */
  // 0-high range, 1-low range, 2-medium range
  void cc_range() const
  {
    const char * cmd = "STATic:CC:RANGe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_range(unsigned v)
  {
    char cmd[sizeof("STATic:CC:RANGe X")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:RANGe %u", v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    cc_range_ = v;
  }

  void cc_xx_level(unsigned range) const
  {
    char cmd[sizeof("STATic:CC:xxxx:LEVel?")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel?", el_range_to_string(range));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_level() const
  {
    cc_xx_level(cc_range_);
  }

  /*
    u2_lf STATic:CC:HIGH:LEVel 1.12345 OK
    u2_lf STATic:CC:HIGH:LEVel 12.1234 OK
    u2_lf STATic:CC:HIGH:LEVel 123.123 OK
    u2_lf STATic:CC:HIGH:LEVel 1234.12 NG
  */
  void cc_xx_level(unsigned range, double v) const
  {
    char cmd[sizeof("STATic:CC:xxxx:LEVel 123.123")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel %.5f", el_range_to_string(range), v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_level(double v) const
  {
    cc_xx_level(cc_range_, v);
  }

  /*
    Constant Voltage

    Example: Set the load to the constant voltage function, and the main value of
    the constant voltage to 30V. The programming command is as follows:

    INPut:FUNCtion CV             // Switch to the constant voltage test function
    STATic:CV:RANGe 0             // Set the constant current range to the high range
    STATic:CV:RATe 0              // Set the constant voltage response speed
    STATic:CV:HIGH:LEVel 50.0     // Set the load voltage to 50.0V
    INPut:STATe ON                // Turn on the load input
  */
  // 0-high range, 1-low range, 2-medium range
  void cv_range() const
  {
    const char * cmd = "STATic:CV:RANGe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_range(unsigned v)
  {
    char cmd[sizeof("STATic:CV:RANGe X")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:RANGe %u", v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    cv_range_ = v;
  }

  void cv_xx_level(unsigned range) const
  {
    char cmd[sizeof("STATic:CV:xxxx:LEVel?")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel?", el_range_to_string(range));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_level() const
  {
    cv_xx_level(cv_range_);
  }

/*
    u2_lf STATic:CV:HIGH:LEVel?
    u2_lf STATic:CV:HIGH:LEVel 1.13245 OK
    u2_lf STATic:CV:HIGH:LEVel 12.1324 OK
    u2_lf STATic:CV:HIGH:LEVel 123.123 OK
    u2_lf STATic:CV:HIGH:LEVel 1234.13 NG
*/
  void cv_xx_level(unsigned range, double v) const
  {
    char cmd[sizeof("STATic:CV:xxxx:LEVel 123.123")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel %.5f", el_range_to_string(range), v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_level(double v) const
  {
    cv_xx_level(cv_range_, v);
  }

  void get_input() const
  {
    const char * cmd = "INPut:STATe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void set_input_ON() const
  {
    const char * cmd = "INPut:STATe ON";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void set_input_OFF() const
  {
    const char * cmd = "INPut:STATe OFF";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

#if 0
  // SYST:LOC // Exit remote control mode and return to local control mode.
  void release_control() const
  {
    //const char * cmd = "SYST:LOC";
    const char * cmd = "SYSTem:LOCal";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }
#endif

  void clear_errors() const
  {
    const char * cmd = "*CLS";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void id() const
  {
    const char * cmd = CMD_IDENTIFY;
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }
};

extern EL_Device elDevice;

struct Test
  : Runnable
{
  unsigned step_;
  unsigned sub_step_;
  //uint32_t start_time_ms_;
  uint32_t event_time_ms_;

  bool run_;
  bool abort_;

  enum class State {
    init,
    idle,
    running,
  };
  static const char * to_string(State state);

  void state(State new_state)
  {
    const auto old_state_name = to_string(state_);
    state_ = new_state;
    const auto new_state_name = to_string(state_);
    LOG("'%s' -> '%s'", old_state_name, new_state_name);
  }

  State state_ = State::init;

  void on_state_init(uint32_t now_ms)
  {
    state(State::idle);
  }


  void change_PS_EL_configs(const ProfileStep * profileStep)
  {
    if (ELState::off == profileStep->elconfig_.state_) {
      LOG("deactivatin EL");
      elDevice.set_input_OFF();
    } else if (ELState::CC == profileStep->elconfig_.state_) {
      LOG("changing EL config");
      elDevice.cc_range(profileStep->elconfig_.range_);
      elDevice.cc_level(profileStep->elconfig_.level_);
      elDevice.cc_level();
      elDevice.set_input_ON();
    } else {
      LOG("ERROR: Unexpected EL state");
    }
  }

  void deactivate_PS_EL()
  {
    LOG("deactivating EL");
    elDevice.set_input_OFF();
  }


  void on_state_idle(uint32_t now_ms)
  {
    if (run_) {
      run_ = false;
      abort_ = false;

      if (!profile.steps_count_) {
        LOG("ERROR: profile is empty");
        state(State::idle);
      } else {
        state(State::running);

        step_ = 0;
        sub_step_ = 0;

        const ProfileStep * profileStep = &profileSteps_[step_];

        //start_time_ms_ = now_ms;
        event_time_ms_ = now_ms;

        // Change PS and EL configs.
        change_PS_EL_configs(profileStep);

        // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
        // be executed on next 'on_loop'.
        if (profileStep->substeps_count_
        && profileStep->measure_period_ms_) {
          event_time_ms_ += profileStep->measure_period_ms_;
        }
      }
    }
  }

  void on_event(uint32_t now_ms)
  {
    LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);

    const ProfileStep * profileStep = &profileSteps_[step_];
    ++sub_step_;
    if (sub_step_ < profileStep->substeps_count_) {
      event_time_ms_ += profileStep->measure_period_ms_;
    } else {
      LOG("--- step_=%u finished -- change PS and EL configs...", step_);

      ++step_;
      if (step_ < profile.steps_count_) {

        profileStep = &profileSteps_[step_];
        sub_step_ = 0;

        // Change PS and EL configs.
        change_PS_EL_configs(profileStep);

        // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
        // be executed on next 'on_loop'.
        if (profileStep->substeps_count_
        && profileStep->measure_period_ms_) {
          event_time_ms_ += profileStep->measure_period_ms_;
        }

      } else {
        deactivate_PS_EL();
        LOG("--- test finished ---");
        // 'test' finished
        state(State::idle);
      }
    }
  }

  void on_state_running(uint32_t now_ms)
  {
    if (abort_) {
      abort_ = false;

      deactivate_PS_EL();

      state(State::idle);
    } else
    if (event_time_ms_ <= now_ms) {
      on_event(now_ms);
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
      CASE_CALL(init)
      CASE_CALL(idle)
      CASE_CALL(running)
#undef CASE_CALL
      default:
        LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
        break;
    }
  }

  void run()
  {
    run_ = true;
  }

  void abort()
  {
    abort_ = true;
  }
};

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
// 'NGI,N69204-150-400,2605300000_N26051432,V2.49'

EL_Device elDevice = {
  /*.uartConfig =*/ &el_uartConfig,
  /*.expected_id =*/
  //"TENMA 72-13210 V2.50 SN:00012161",
  //"TENMA 72-13210 V2.20 SN:00010724",
  "NGI,N69204-150-400,2605300000_N26051432,V2.49",
};

void el_id(const char * args)
{
  (void) args;
  elDevice.id();
}

/*
void el_release_control(const char * args)
{
  (void) args;
  elDevice.release_control();
}
*/

void el_clear_errors(const char * args)
{
  elDevice.clear_errors();
}

void el_function(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "cc")) {
      elDevice.function_CC();
    } else
    if (!strcasecmp(args, "cv")) {
      elDevice.function_CV();
    } else {
      LOG("function must be CC or CV");
    }
  }
  elDevice.function();
}

void el_input(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "on")) {
      elDevice.set_input_ON();
    } else
    if (!strcasecmp(args, "off")) {
      elDevice.set_input_OFF();
    } else {
      LOG("input must be ON or OFF");
    }
  }
  elDevice.get_input();
}

void el_cc_level(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cc_level(v);
  }
  elDevice.cc_level();
}

void el_cv_level(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cv_level(v);
  }
  elDevice.cv_level();
}

void el_cc_range(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    unsigned v = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cc_range(v);
  }
  elDevice.cc_range();
}

void el_cv_range(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    unsigned v = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cv_range(v);
  }
  elDevice.cv_range();
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

Test test;

const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "idle",
    "running",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void test_run(const char * args)
{
  (void) args;
  test.run();
}

void test_abort(const char * args)
{
  (void) args;
  test.abort();
}

// u1_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u1_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u1_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u1_lf *IDN?

// u2_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u2_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u2_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u2_lf *IDN?
// u2_lfcr *IDN?
// u2_cr *IDN?
// u2_crlf *IDN?
// u2_lf *CLS?

//static
void ux_hex(unsigned n, const char * args)
{
  const UartConfig * uartConfig0 = xx_uartConfigs[0];
  const UartConfig * uartConfig = xx_uartConfigs[n];
  for (;;) {
    args = skip_whitespace(args);
    if (!*args) { break; }
    char * endptr = NULL;
    uint8_t xx = strtoul(args, &endptr, 0);
    LOG(". xx='0x%02X'", xx);

    uartConfig0->write('"');
    uartConfig0->write(xx);
    uartConfig0->write('"');
    uartConfig0->write('\r');
    uartConfig0->write('\n');

    uartConfig->write(xx);
    args = endptr;
  }
}

//static
void ux_cr(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\r');
}

void ux_crlf(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\r');
  uartConfig->write('\n');
}

//static
void ux_lf(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\n');
}

void ux_lfcr(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\n');
  uartConfig->write('\r');
}

void u1_hex(const char * args) { ux_hex(1, args); }
void u2_hex(const char * args) { ux_hex(2, args); }
void u1_cr(const char * args) { ux_cr(1, args); }
void u2_cr(const char * args) { ux_cr(2, args); }
void u1_crlf(const char * args) { ux_crlf(1, args); }
void u2_crlf(const char * args) { ux_crlf(2, args); }
void u1_lf(const char * args) { ux_lf(1, args); }
void u2_lf(const char * args) { ux_lf(2, args); }
void u1_lfcr(const char * args) { ux_lfcr(1, args); }
void u2_lfcr(const char * args) { ux_lfcr(2, args); }

void ux_baud(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];

  char * endptr = NULL;
  uint32_t baudrate = strtoul(args, &endptr, 0);
  if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }
  LOG("setting baudrate=%u", baudrate);
  uartConfig->baudRate(baudrate);
  LOG("active baudrate=%u", uartConfig->baudRate());
}

void u1_baud(const char * args) { ux_baud(1, args); }
void u2_baud(const char * args) { ux_baud(2, args); }

void command_idn(const char * args)
{
  (void) args;
  LOG("NubaCell BT ESP32 v.0.0 sn:1234567890");
}

static const commands_map_t commands_map[] = {
#define CMD(x) { #x, CONSTSTRLEN(#x), x, }
  CMD(u1_hex),
  CMD(u2_hex),
  CMD(u1_cr),
  CMD(u2_cr),
  CMD(u1_crlf),
  CMD(u2_crlf),
  CMD(u1_lf),
  CMD(u2_lf),
  CMD(u1_lfcr),
  CMD(u2_lfcr),
  CMD(u1_baud),
  CMD(u2_baud),
#undef CMD
#define COMMAND(cmd,func) { cmd, CONSTSTRLEN(cmd), func, }
  COMMAND("*IDN?", command_idn),
#undef COMMAND
#define COMMAND(cmd) { #cmd, CONSTSTRLEN(#cmd), cmd, }

  COMMAND(el_id),
  COMMAND(el_clear_errors),
  //COMMAND(el_release_control),
  COMMAND(el_function),
  COMMAND(el_input),

  COMMAND(el_cc_range),
  COMMAND(el_cv_range),
  COMMAND(el_cc_level),
  COMMAND(el_cv_level),

  COMMAND(profile_steps),
  COMMAND(profile_steps_dump),
  COMMAND(profile_step),

  COMMAND(test_run),
  COMMAND(test_abort),
#undef COMMAND
};

static
void
print_commands(
    const commands_map_t * commands_map,
    size_t commands_count)
{
  LOG("Available commands:");
  for (unsigned i = 0; i < commands_count; ++i) {
    LOG(". %s", commands_map[i].command_s);
  }
}

void print_commands()
{
  print_commands(commands_map, ARRAY_SIZE(commands_map));
}

static
command_fn_t
command_s_to_fn(
    const commands_map_t * commands_map,
    size_t commands_count,
    const char * command_s,
    size_t command_len)
{
  for (unsigned i = 0; i < commands_count; ++i) {
    if (commands_map[i].command_len == command_len
    && !strncmp(command_s, commands_map[i].command_s, command_len)) {
      return commands_map[i].command_fn;
    }
  }
  return NULL;
}

command_fn_t
command_s_to_fn(
    const char * command_s,
    size_t command_len)
{
  return command_s_to_fn(
      commands_map, ARRAY_SIZE(commands_map),
      command_s, command_len);
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

  uartDevices_on_loop(now_ms);
  //controller.on_loop(now_ms);
  test.on_loop(now_ms);

  ++loop_count;
}

// TODO: Use 'enum class' instead of 'unsigned'

const char * const el_functions[] = { "CC", "CV", };

const char * el_function_to_string(unsigned function)
{
  return el_functions[function];
}


const char * const el_ranges[] = { "HIGH", "LOW", "MID", };

const char * el_range_to_string(unsigned cc_range)
{
  return el_ranges[cc_range];
}
