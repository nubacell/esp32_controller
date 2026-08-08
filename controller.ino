#include <Arduino.h>

// https://github.com/RobTillaart/CRC
#include "CRC32.h"

/*
  The following function declaration shall be automatically
  taken from #include "CRC32.h" above but build fails if it
  is not repeated here.
*/
uint8_t calcCRC8(
    const uint8_t *array, crc_size_t length,
    uint8_t polynome   = CRC8_POLYNOME,
    uint8_t initial    = CRC8_INITIAL,
    uint8_t xorOut     = CRC8_XOR_OUT,
    bool reverseIn     = CRC8_REV_IN,
    bool reverseOut    = CRC8_REV_OUT,
    crc_size_t yieldPeriod = CRC_YIELD_DISABLED);

#include "driver/gpio.h"

extern "C" {
esp_err_t gpio_output_enable(gpio_num_t gpio_num);
}

#include "driver/uart.h"
#include "esp_err.h" // ESP_ERROR_CHECK()
#include "esp_log.h" // ESP_LOG*()

#include "driver/spi_slave.h"
#include "esp_private/spi_slave_internal.h" // spi_slave_queue_reset()

//#define COLLECT_MIN_MAX

#define CN_UART_NUM             0
//#define CN_UART_BAUD            115200
#define CN_UART_BAUD            3000000

#define PS_UART_NUM             1
#define PS_UART_BAUD            9600
#define PS_UART_RX_GPIO         16
#define PS_UART_TX_GPIO         17

#define EL_UART_NUM             2
//#define EL_UART_BAUD            115200
#define EL_UART_BAUD            38400
#define EL_UART_RX_GPIO         26
#define EL_UART_TX_GPIO         25

#define OFFSET_OF(container_type, member_name) \
    ((size_t)&(((container_type *)0)->member_name))

#define SIZE_OF_MEMBER(container_type, member_name) \
    sizeof((((container_type *)0)->member_name))

#define OFFSET_OF_END(container_type, member_name) \
    (OFFSET_OF(container_type, member_name) \
        + SIZE_OF_MEMBER(container_type, member_name))

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#define CONSTSTRLEN(s) (sizeof(s)-1)

#define MAKE_STRING_HELPER2(s) #s
#define MAKE_STRING_HELPER1(s) MAKE_STRING_HELPER2(s)
#define MAKE_STRING(s) MAKE_STRING_HELPER1(s)

uint32_t cpu_sp_get();

void hex_dump(const uint8_t * data, size_t size);

static void log(const char *file, unsigned line, const char * function, const char * format, ...);
//#define LOG(format, args...) log(__FILE__, __LINE__, __FUNCTION__, format, ##args)
#define LOG(format, args...) log(__FILE__, __LINE__, __PRETTY_FUNCTION__, format, ##args)

void log_write(unsigned line, const char * function, const char * s);

//__attribute__((noinline))
__attribute__((noipa))
void stack_recurse(unsigned depth);

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump(unsigned line, const char * func);
#define STACK_DUMP() stack_dump(__LINE__, __FUNCTION__)

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump_recursive(unsigned line, const char * func, unsigned depth);
#define STACK_DUMP_RECURSIVE(depth) stack_dump_recursive(__LINE__, __FUNCTION__, depth)

__attribute__((noinline))
void stack_marker_fill(uint32_t * addr, size_t size, uint8_t fill);
#define STACK_MARKER(size,fill) \
    { \
      uint32_t stack_marker[size/sizeof(uint32_t)]; \
      stack_marker_fill(stack_marker, sizeof(stack_marker), fill); \
    }

extern const char device_tag[];

extern const char CMD_IDENTIFY[];
#pragma once

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // [u]int(8|16|32|64)_t

#define CVT_HEX_MAX_DIGITS  16
/*  8 = roundUp(32bits/4) */
/* 16 = roundUp(64bits/4) */
#define CVT_UDEC_MAX_DIGITS 20
/* 10 = roundUp(32bits*3/10) */
/* 20 = roundUp(64bits*3/10) */
#define CVT_SDEC_MAX_DIGITS (1+CVT_UDEC_MAX_DIGITS)
#define CVT_BINARY_MAX_DIGITS 64

size_t cvt_ud2a(char* buffer, uint64_t number, uint8_t base, bool upperCase);
size_t cvt_hex (char* buffer, uint64_t number, bool upperCase);
size_t cvt_uDec(char* buffer, uint64_t number);
size_t cvt_sDec(char* buffer,  int64_t number);
size_t cvt_oct (char* buffer, uint64_t number);
size_t cvt_bin (char* buffer, uint64_t number);

// Warning: Don't forget to provide extra space for zero terminator.
// Note: Returned value does not include zero terminator.
size_t cvt_ud2a_z(char* buffer, uint64_t number, uint8_t base, bool upperCase);
size_t cvt_hex_z (char* buffer, uint64_t number, bool upperCase);
size_t cvt_uDec_z(char* buffer, uint64_t number);
size_t cvt_sDec_z(char* buffer,  int64_t number);
size_t cvt_oct_z (char* buffer, uint64_t number);
size_t cvt_bin_z (char* buffer, uint64_t number);

size_t cvt_u32tos(char* buffer, uint32_t number, uint8_t base, bool upperCase);

size_t safe_vsnprintf(char * buffer, size_t size, const char * format, va_list ap);
size_t safe_snprintf(char * buffer, size_t size, const char * format, ...);

int sign(int v)
{
  return (v < int(0)) ? -1 : (v > int(0)) ? +1 : 0;
}

const char * skip_whitespace(const char * p);
char * skip_whitespace(char * p);

struct Runnable
{
  virtual void on_loop(uint32_t now_ms)
  {
    LOG("");
  }
};

int32_t command_args_to_i32(const char * args, const char ** tail);

uint32_t command_args_to_u32(const char * args, const char ** tail);

double command_args_to_ud(const char * args, const char ** tail);

#define command_args_to_ul command_args_to_u32

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

void el_id(const char * args);
void el_clear_errors(const char * args);
void el_function(const char * args);
void el_input(const char * args);
void el_cc_level(const char * args);
void el_cv_level(const char * args);
void el_cc_range(const char * args);
void el_cv_range(const char * args);

void ps_id(const char * args);
void ps_v(const char * args);
void ps_c(const char * args);
void ps_ovp(const char * args);
void ps_ocp(const char * args);
void ps_output(const char * args);

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

extern const UartConfig cn_uartConfig;

extern const UartConfig * const xx_uartConfigs[];

void xx_uartConfigs_init();

// TODO: Use 'enum class' instead of 'unsigned'

extern const char * const el_functions[];
const char * el_function_to_string(unsigned function);

extern const char * const el_ranges[];
const char * el_range_to_string(unsigned cc_range);

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

#define XX_LOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct XX_Device
  : UartDevice
{
  const char * const expected_id;
  uint32_t timeout_time_ms = 0;

#if 0
  bool cmd_verbose_ = false;
#else
  bool cmd_verbose_ = true;
#endif

#if 0
  bool cmd_verify_ = false;
#else
  bool cmd_verify_ = true;
#endif

  bool on_ = false;
  inline bool isOn() const { return on_; }

  XX_Device(
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
    if (cmd_verbose_) {
      XX_LOG("cmd='%s'", s);
    }
  }

#if 0
  void command_try(
    const char * const command,
    unsigned answer_lines_count)
  {
    write_cmd(command);
    while (answer_lines_count--) {
      const uint32_t timeout_ms = 100;
      if (!uartConfig_->readLine_z(buffer_, sizeof(buffer_), timeout_ms)) {
        XX_LOG("'%s' -> '%s'", command, "timeout");
      } else {
        XX_LOG("'%s' -> '%s'", command, buffer_);
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
#endif

  void on_state_init(uint32_t now_ms)
  {
    XX_LOG("");
    state = State(unsigned(state)+1); // ++state;
  }

  void on_state_sync(uint32_t now_ms)
  {
    XX_LOG("");
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
    //XX_LOG("");
    if (on_loop_readLine()) {
      XX_LOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_readLine_reset();
    }
    if (timeout_time_ms <= now_ms) {
      XX_LOG("%s %s", "sync/discard_input", "timeout/success");
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_identify(uint32_t now_ms)
  {
    XX_LOG("");
    on_loop_readLine_reset();

    write_cmd(CMD_IDENTIFY);

    constexpr uint32_t identify_timeout_ms = 120;
    timeout_time_ms = now_ms + identify_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  // assuming 'sizeof(buffer) > 0'
  void on_state_identify_read_answer(uint32_t now_ms)
  {
    //XX_LOG("");
    if (on_loop_readLine()) {
      XX_LOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      if (eol_) {

        // TODO: check id match

        state = State(unsigned(state)+1); // ++state;
      } else if (overflow_) {

        // TODO: Mark device as unusable.
        // TODO: Retry identification after some delay.

        state = State(unsigned(state)+1); // ++state;
      }
    } else if (timeout_time_ms <= now_ms) {
      XX_LOG("%s %s id='%s'", "identify", "timeout", buffer_);

      // TODO: Mark device as unusable.
      // TODO: Retry identification after some delay.

      //state = State::identify;
      //state = State::init;
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_idle(uint32_t now_ms)
  {
    //XX_LOG("");
    if (on_loop_readLine()) {
      XX_LOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_readLine_reset();
    }
  }

  void on_state_active(uint32_t now_ms)
  {
    XX_LOG("");
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
        XX_LOG("");
        break;
    }
  }

  // "*IDN?"
  void id() const
  {
    const char * cmd = CMD_IDENTIFY;
    write_cmd(cmd);
  }
};

struct PS_Device
  : XX_Device
{
  PS_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : XX_Device(uartConfig_, expected_id_)
  {}

  void ovp() const;
  void ocp() const;
  void output() const;

  void x(const char * s, double v) const;

  void ovp(double v) const;
  void ocp(double v) const;

  void on();
  void off();

  void v() const;
  void v(double v) const;
  void c() const;
  void c(double v) const;
};

extern PS_Device psDevice;

struct EL_Device
  : XX_Device
{
  uint8_t function_;
  uint8_t cc_range_;
  uint8_t cv_range_;

  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : XX_Device(uartConfig_, expected_id_)
  {}

  void clear_errors() const;

  void function() const;
  void function_xx(unsigned v);
  void function_CC();
  void function_CV();

  // 0-high range, 1-low range, 2-medium range
  void cc_range() const;
  void cc_range(unsigned v);
  void cc_xx_level(unsigned range) const;
  void cc_level() const;
  void cc_xx_level(unsigned range, double v) const;
  void cc_level(double v) const;

  // 0-high range, 1-low range, 2-medium range
  void cv_range() const;
  void cv_range(unsigned v);
  void cv_xx_level(unsigned range) const;
  void cv_level() const;
  void cv_xx_level(unsigned range, double v) const;
  void cv_level(double v) const;
  void get_input() const;

  void on();
  void off();

  void c() const;
  void c(double v);
};

extern EL_Device elDevice;

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

void pin_dump(const char * args);
void pin_reset(const char * args);
void pin_level(const char * args);
void pin_mode(const char * args);
void pin_pull(const char * args);

enum class PSState {
  off,
  CC,
  CV,
};
const char * to_string(PSState psstate);

// Note: We age going to use "EL" in "CC" mode only

enum class ELState {
  off,
  CC,
  CV,
};
const char * to_string(ELState elstate);

struct ProfileStep
{
  uint32_t step_ms;
  uint32_t sub_step_ms;
   int32_t I_A;
  uint32_t PS_V_max;
  uint32_t cBat_max_mV;
  uint32_t cBat_min_mV;
  uint32_t aBat_max_mV;
  uint32_t aBat_min_mV;
  uint32_t cBat_T_max_C;
  uint32_t aBat_T_max_C;

  void dump() const;
  void apply(const char * args);
};

constexpr uint32_t profile_steps_count_max = 100;
static ProfileStep profileSteps_[profile_steps_count_max];

struct Profile
{
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
      LOG("new_steps_count=%u > profile_steps_count_max=%u",
          new_steps_count, profile_steps_count_max);
    } else {
      steps_count_ = new_steps_count;
    }
  }
};

Profile profile;

void serial_write_buffer(
    const char *file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used);

#define SERIAL_WRITE_BUFFER(buffer, buffer_size, used) \
    serial_write_buffer( \
        __FILE__, __LINE__, __PRETTY_FUNCTION__, \
        buffer, buffer_size, used);

bool spi_chip_init();

#define SPI_GPIO_MISO 12
#define SPI_GPIO_MOSI 13
#define SPI_GPIO_SCLK 14
#define SPI_GPIO_CS   15

#define SPI_DEVICE SPI2_HOST

#define SPI_BUFFER_SIZE_MAX 64
#define SPI_BUFFER_SIZE SPI_BUFFER_SIZE_MAX
//#define SPI_BUFFER_SIZE 128
#define SPI_DATA_SIZE 29


#define SPI_SENSOR_DEVICES_COUNT 8

// 'Master Request' (MR) multiplexer (MUX) is used to select one of many LGT8F
// devices
#define SPI_MR_MUX_GPIO_COUNT 4
#define SPI_MR_MUX_0_GPIO 32
#define SPI_MR_MUX_1_GPIO 33
#define SPI_MR_MUX_2_GPIO 19
#define SPI_MR_MUX_3_GPIO 18

// 'Master Request' (MR) is used to request SPI transfer from one of many LGT8F
// devices selected by 'MR_MUX'
#define SPI_MR_GPIO 4

extern const uint8_t spi_mr_mux_pins[SPI_MR_MUX_GPIO_COUNT];

extern byte spi_tx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
extern byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
extern uint16_t spi_rx_bits[SPI_SENSOR_DEVICES_COUNT];

extern const spi_bus_config_t spi_bus_config;
extern const spi_slave_interface_config_t spi_slave_interface_config;


// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
void spi_post_setup_cb(spi_slave_transaction_t * transaction);

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
void spi_post_trans_cb(spi_slave_transaction_t * transaction);

void spi_setup();

void spi_mr_pin_setup();
void spi_mr_mux_pins_setup();
void spi_mr_mux(unsigned spi_sensor_n);
void spi_pins_setup();

bool spi_transfer_start(uint8_t spi_sensor_n);

void spi_rx_bits_reset();
bool spi_rx_bits_valid();


struct SPI_Device
  : Runnable
{
  uint32_t transfer_start_time_ms_;
  uint32_t transfer_finish_time_ms_;
  uint32_t timeout_time_ms_;

  bool run_ = false;
  bool sensors_data_valid_ = false;

  enum class State {
    init,
    init_failed,
    idle,
    running,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_running(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  void run()
  {
    run_ = true;
  }

  bool isTransferFinished() { return State::idle == state_; }
  bool areSensorsDataValid() { return sensors_data_valid_; }
};

extern SPI_Device spiDevice;


extern unsigned spi_post_setup_cb_count_;
extern unsigned spi_post_trans_cb_count_;

void spi_report_stats();

void spi_run(const char * args);
void spi_stats(const char * args);

#define voltage_pins_count 4
#define temperature_pins_count 4
#define temperature_groups_count 2

typedef uint16_t AnalogSignal_t;
#define ANALOG_SIGNALS_COUNT 12
typedef union AnalogSignals_u {
  struct {
    AnalogSignal_t voltage[voltage_pins_count];
    AnalogSignal_t temperature[temperature_groups_count][temperature_pins_count];
  };
  AnalogSignal_t all[ANALOG_SIGNALS_COUNT];
} AnalogSignals_t;

typedef uint16_t VCC_t;

typedef struct SensorDeviceData_s {
  uint8_t format; // data format version number, 1 for now
  uint8_t device_id;
  AnalogSignals_t analogSignals;
  VCC_t vcc;
  uint8_t checksum;
} SensorDeviceData_t;
/*
    sizeof(SensorDeviceData_t) on
      LGT8F  21
      ESP32  22 one extra byte is added after checksum
*/
#define LGT8F_SPI_DATA_SIZE OFFSET_OF_END(SensorDeviceData_t, checksum)

void analog_report_one(const SensorDeviceData_t & deviceData, unsigned i);
void analog_report_all(const SensorDeviceData_t & deviceData);
void SensorDevice_Data_report_one(unsigned i);
void SensorDevices_Data_report_all();

struct Test
  : Runnable
{
  unsigned step_;
  unsigned sub_step_;
  uint32_t test_start_time_ms_;
  uint32_t step_end_time_ms_;
  uint32_t sub_step_end_time_ms_;
  uint32_t spi_timeout_time_ms_;

  int32_t I_A = 0;
  uint32_t PS_V_max = 0;

  bool run_;
  bool abort_;

  enum class State {
    idle,
//  init,                   // start_time_ms_ = sub_step_end_time_ms_ = now_ms;
//  step_0,                 // step_ = 0;
    step_init,              // 1 set PS/EL configs
//       sub_step_0,        // sub_step_ = 0;
         sub_step_init,     // 2 'sub_step_end_time_ms_ += measure_period_ms_;'
         sub_step_wait,     // 3 wait for end of sub step (sub_step_end_time_ms_)
         sub_step_measure,
//                spi_init, // start SPI transfer to get measurements
                  spi_wait, // 4 wait for end of SPI transfer
         sub_step_next,     // 5 '++sub_step_ < substeps_count_ ? sub_step_init : step_next'
    step_next,              // 6 '++step_ < steps_count_ ? step_init : cleanup'
    abort,                  // 7 report and cleanup (deactivate PS and EL)
    cleanup,                // 8 deactivate PS and EL
  };
  State state_ = State::idle;
  static const char * to_string(State state);

  void state(State new_state)
  {
    const auto old_state_name = to_string(state_);
    state_ = new_state;
    const auto new_state_name = to_string(state_);
    LOG("'%s' -> '%s'", old_state_name, new_state_name);
  }

  void change_PS_EL_configs(const ProfileStep * profileStep);
  void deactivate_PS_EL();

  void on_loop(uint32_t now_ms) override;

  void on_state_idle(uint32_t now_ms);
  void on_state_step_init(uint32_t now_ms);
  void on_state_sub_step_init(uint32_t now_ms);
  void on_state_sub_step_wait(uint32_t now_ms);
  void on_state_sub_step_measure(uint32_t now_ms);
  void on_state_spi_wait(uint32_t now_ms);
  void on_state_sub_step_next(uint32_t now_ms);
  void on_state_step_next(uint32_t now_ms);
  void on_state_abort(uint32_t now_ms);
  void on_state_cleanup(uint32_t now_ms);

  void run()
  {
    run_ = true;
  }

  void abort()
  {
    abort_ = true;
  }

  void dump() const;
};

uint32_t cpu_sp_get()
{
  register uint32_t rsp asm("sp");
  return rsp;
}

static char nibble_hex(uint8_t b)
{
  uint8_t n = b & 0x0F;
  return (n < 10) ? '0' + n : 'A' - 10 + n;
}

static void hex_dump_byte(char * p, uint8_t b)
{
  p[0] = nibble_hex(b >> 4);
  p[1] = nibble_hex(b);
}

static char make_printable(uint8_t b)
{
  b &= 0x7F;
  return (b < 0x20 || b >= 0x7F) ? '.' : b;
}

void hex_dump_line(
    char * hex_s, size_t hex_s_size,
    char * char_s, size_t char_s_size,
    const uint8_t * data, size_t size)
{
  memset(hex_s, ' ', hex_s_size - 1);
  hex_s[hex_s_size - 1] = '\0';
  memset(char_s, ' ', char_s_size - 1);
  char_s[char_s_size - 1] = '\0';
  for (unsigned gi = 0; gi < 4; ++gi) {
    for(unsigned bi = 0; bi < 4; ++bi) {
      unsigned di = gi * 4 + bi;
      unsigned hex_s_i = gi * (2 + 4*2+3) + bi * (2+1);
      unsigned char_s_i = gi * (1 + 4) + bi;
      if (di < size) {
        uint8_t d = data[di];
        hex_dump_byte(&hex_s[hex_s_i], d);
        char_s[char_s_i] = make_printable(d);
      } else {
        hex_s[hex_s_i  ] = '.';
        hex_s[hex_s_i+1] = '.';
        char_s[char_s_i] = '.';
      }
    }
  }
}

// "XXXXXXXX/XXX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XXXX XXXX XXXX XXXX"
void hex_dump(const uint8_t * data, size_t size)
{
  //LOG("data=%p size=%zu", data, size);
  static
  char s[sizeof("XXXXXXXX/XXX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XXXX XXXX XXXX XXXX")];
  for (uint32_t offs = 0; offs < size; offs += 16) {
    const uint8_t * addr = data + offs;
    static
    char hex_s[sizeof("XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX")];
    static
    char char_s[sizeof("XXXX XXXX XXXX XXXX")];
    size_t left = size - offs;
    hex_dump_line(hex_s, sizeof(hex_s), char_s, sizeof(char_s), addr, left);
    //LOG("hex_s='%s'", hex_s);
    //LOG("char_s='%s'", char_s);
    snprintf(s, sizeof(s), "%08X/%03X  %s  %s", addr, offs, hex_s, char_s);
    Serial.print(". . ");
    Serial.println(s);
  }
}

void log(const char *file, unsigned line, const char * function, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);

  constexpr size_t buffer_size = 172;
  static char buffer[buffer_size];
  const int vsnprintf_r = vsnprintf(buffer, sizeof(buffer), format, argsList);
  log_write(line, function, buffer);

  va_end(argsList);

  if (vsnprintf_r < 0) {
    const int snprintf_r = snprintf(buffer, sizeof(buffer),
        "ALERT: '%s()' failure, errno=%i", "vsnprintf", errno);
    if (snprintf_r < 0) {
      log_write(__LINE__, __FUNCTION__, "ALERT: 'snprintf()' failure");
    } else {
      log_write(__LINE__, __FUNCTION__, buffer);
    }
  } else {
    const size_t output_length = (size_t) vsnprintf_r;
    const size_t needed_size = output_length + 1; // add space for terminating '\0'
    if (buffer_size < needed_size) {
      const int snprintf_r = snprintf(buffer, sizeof(buffer),
          "ALERT: logger buffer overflow (needed=%zu available=%zu)",
          needed_size, sizeof(buffer));
      if (snprintf_r < 0) {
        log_write(__LINE__, __FUNCTION__, "ALERT: 'snprintf()' failure");
      } else {
        log_write(__LINE__, __FUNCTION__, buffer);
      }
    }
  }
}

void log_write(unsigned line, const char * function, const char * s)
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

uint32_t regs_[8];
//__attribute__((noinline))
__attribute__((noipa))
void save_regs(
    uint32_t r0,
    uint32_t r1,
    uint32_t r2,
    uint32_t r3,
    uint32_t r4,
    uint32_t r5)
{
  regs_[ 0] = r0;
  regs_[ 1] = r1;
  regs_[ 2] = r2;
  regs_[ 3] = r3;
  regs_[ 4] = r4;
  regs_[ 5] = r5;
}

unsigned stack_recurse_count_;
//__attribute__((noinline))
__attribute__((noipa))
void stack_recurse(unsigned depth)
{
  asm volatile ("");
  STACK_MARKER(16, '1');
  if (depth) {
    stack_recurse(depth - 1);
  }
#if 1
  {
    save_regs(
        cpu_sp_get() & 0xFFFFFF00,
        1,
        2,
        3,
        4,
        5);
  }
  ++stack_recurse_count_;
#endif
}

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump_recursive(unsigned line, const char * func, unsigned depth)
{
  asm volatile ("");
  STACK_MARKER(16, '2');
  stack_recurse(4);
  //const void * p = &p;
  //LOG("_ %u:%s p=%p", line, func, p);
  //LOG("_ %u:%s sp=%p p=%p", line, func, cpu_sp_get(), p);
  LOG("_ %u:%s sp=%p", line, func, cpu_sp_get());
  if (depth) {
    stack_dump_recursive(line, func, depth - 1);
  } else {
    stack_dump(line, func);
  }
  ++stack_recurse_count_;
}

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump(unsigned line, const char * func)
{
  asm volatile ("");
  STACK_MARKER(16, '3');
  //stack_recurse(4);
  //const void * p = &p;
  //LOG("_ %u:%s p=%p", line, func, p);
  //LOG("_ %u:%s sp=%p p=%p", line, func, cpu_sp_get(), p);
  LOG("_ %u:%s sp=%p", line, func, cpu_sp_get());
  //hex_dump((const uint8_t*)p, 256);
  //hex_dump( ((const uint8_t*)p) - 32, 256);
  hex_dump( ((const uint8_t*)cpu_sp_get()) - 256, 256*3);
}

__attribute__((noinline))
void stack_marker_fill(uint32_t * addr, size_t size, uint8_t fill)
{
  memset(addr, fill, size);
  if (size >= sizeof(uint32_t)) {
    *addr = uint32_t(addr);
  }
}

const char device_tag[] = "esp32";

size_t
cvt_bin(char* buffer, uint64_t number)
{
  return cvt_ud2a(buffer, number, 2U, false);
}

size_t
cvt_bin_z(char* buffer, uint64_t number)
{
  size_t size = cvt_bin(buffer, number);
  buffer[size] = '\0';
  return size;
}

size_t
cvt_hex(char* buffer, uint64_t number, bool upperCase)
{
  return cvt_ud2a(buffer, number, 16U, upperCase);
}

size_t
cvt_hex_z(char* buffer, uint64_t number, bool upperCase)
{
  size_t size = cvt_hex(buffer, number, upperCase);
  buffer[size] = '\0';
  return size;
}

size_t
cvt_oct(char* buffer, uint64_t number)
{
  return cvt_ud2a(buffer, number, 8U, false);
}

size_t
cvt_oct_z(char* buffer, uint64_t number)
{
  size_t size = cvt_oct(buffer, number);
  buffer[size] = '\0';
  return size;
}

size_t
cvt_sDec(char* buffer, int64_t number)
{
  if (number < 0) {
    *buffer++ = '-';
    number = -number;
  }
  return cvt_ud2a(buffer, number, 10U, false);
}

size_t
cvt_sDec_z(char* buffer, int64_t number)
{
  size_t size = cvt_sDec(buffer, number);
  buffer[size] = '\0';
  return size;
}

// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_u32tos(char* buffer, uint32_t number, uint8_t base, bool upperCase)
{
  if (!number)
  {
    *buffer = '0';
    return 1;
  }

  uint8_t digits[CVT_BINARY_MAX_DIGITS];
  size_t digitIndex = 0;

  while (number)
  {
    digits[digitIndex++] = number % base;
    number /= base;
  }
  size_t size = digitIndex;
  while (digitIndex)
  {
    uint8_t ASCIIdigit = digits[--digitIndex];
    if (ASCIIdigit < 10)
    {
      ASCIIdigit += '0';
    }
    else
    {
      ASCIIdigit += upperCase ? ('A' - 10) : ('a' - 10);
    }
    *buffer++ = ASCIIdigit;
  }
  return size;
}

// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_u32tos_z(char* buffer, uint32_t number, uint8_t base, bool upperCase)
{
  size_t size = cvt_u32tos(buffer, number, base, upperCase);
  buffer[size] = '\0';
  return size;
}

// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_ud2a(char* buffer, uint64_t number, uint8_t base, bool upperCase)
{
  if (!number)
  {
    *buffer = '0';
    return 1;
  }

  uint8_t digits[CVT_BINARY_MAX_DIGITS];
  size_t digitIndex = 0;

  while (number)
  {
    digits[digitIndex++] = number % base;
    number /= base;
  }
  size_t size = digitIndex;
  while (digitIndex)
  {
    uint8_t ASCIIdigit = digits[--digitIndex];
    if (ASCIIdigit < 10)
    {
      ASCIIdigit += '0';
    }
    else
    {
      ASCIIdigit += upperCase ? ('A' - 10) : ('a' - 10);
    }
    *buffer++ = ASCIIdigit;
  }
  return size;
}

// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_ud2a_z(char* buffer, uint64_t number, uint8_t base, bool upperCase)
{
  size_t size = cvt_ud2a(buffer, number, base, upperCase);
  buffer[size] = '\0';
  return size;
}

size_t
cvt_uDec(char* buffer, uint64_t number)
{
  return cvt_ud2a(buffer, number, 10U, false);
}

size_t
cvt_uDec_z(char* buffer, uint64_t number)
{
  size_t size = cvt_uDec(buffer, number);
  buffer[size] = '\0';
  return size;
}

size_t safe_vsnprintf(char * buffer, size_t size, const char * format, va_list ap)
{
  const auto r = vsnprintf(buffer, size, format, ap);
  return (r < 0) ? 0 : r;
}

size_t safe_snprintf(char * buffer, size_t size, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);
  size_t r = safe_vsnprintf(buffer, size, format, argsList);
  va_end(argsList);
  return r;
}

const char CMD_IDENTIFY[] = "*IDN?";

int32_t command_args_to_i32(const char * args, const char ** tail)
{
  unsigned v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!*args) {
    v = 0;
  } else {
    v = strtol(args, &endptr, 0);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (endptr) {
    endptr = skip_whitespace(endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (tail) {
    *tail = endptr;
  } else if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }

  return v;
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

uint32_t command_args_to_u32(const char * args, const char ** tail)
{
  unsigned v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!*args) {
    v = 0;
  } else {
    v = strtoul(args, &endptr, 0);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (endptr) {
    endptr = skip_whitespace(endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (tail) {
    *tail = endptr;
  } else if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }

  return v;
}

double command_args_to_ud(const char * args, const char ** tail)
{
  double v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!*args) {
    v = 0;
  } else {
    v = strtod(args, &endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (endptr) {
    endptr = skip_whitespace(endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (tail) {
    *tail = endptr;
  } else if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }

  return v;
}

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
  COMMAND(test_dump),
  COMMAND(test_state),

  COMMAND(spi_run),
  COMMAND(spi_stats),

  COMMAND(ps_id),
  COMMAND(ps_v),
  COMMAND(ps_c),
  COMMAND(ps_ovp),
  COMMAND(ps_ocp),
  COMMAND(ps_output),

  //COMMAND(scope_sync_on),
  //COMMAND(scope_sync_off),
  //COMMAND(scope_sync_pulse),

  COMMAND(pin_dump),
  COMMAND(pin_reset),
  COMMAND(pin_level),
  COMMAND(pin_mode),
  COMMAND(pin_pull),
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

Controller controller;

void pin_dump(const char * args)
{
  unsigned pin = command_args_to_ul(args, nullptr);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }
  gpio_dump_io_configuration(stdout, (1ULL << pin));
}

/*
    esp_err_t gpio_reset_pin(gpio_num_t gpio_num)

      gpio_intr_disable(gpio_num);
      gpio_pullup_en(gpio_num);
      gpio_pulldown_dis(gpio_num);
      gpio_input_disable(gpio_num);
      gpio_output_disable(gpio_num);
      gpio_hal_func_sel(gpio_context.gpio_hal, gpio_num, PIN_FUNC_GPIO);

*/
void pin_reset(const char * args)
{
  unsigned pin = command_args_to_ul(args, nullptr);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }
  if (ESP_OK != gpio_reset_pin(gpio_num_t(pin))) {
    LOG("ERROR: %s(pin=%u)", "gpio_reset_pin", pin);
  }
}

/*
    int gpio_get_level(gpio_num_t gpio_num);
    esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level);

      level Output level. 0: low ; 1: high
*/
void pin_level(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    unsigned v = command_args_to_ul(tail, nullptr);
    LOG(". setting pin=%u level=%u", pin, v);
    if (ESP_OK != gpio_set_level(gpio_num_t(pin), v)) {
      LOG("ERROR: %s(pin=%u)", "gpio_set_level", pin);
    }
  }

  LOG("%s(pin=%u)=%u", "gpio_get_level", pin, gpio_get_level(gpio_num_t(pin)));
}

/*
    esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode);

    gpio_mode_t

      GPIO_MODE_DISABLE
      GPIO_MODE_INPUT                   // (1<<0)
      GPIO_MODE_OUTPUT                  //          (1<<1)
      GPIO_MODE_OUTPUT_OD               //          (1<<1) | (1<<2)
      GPIO_MODE_INPUT_OUTPUT            // (1<<0) | (1<<1)
      GPIO_MODE_INPUT_OUTPUT_OD         // (1<<0) | (1<<1) | (1<<2)

    if (mode & GPIO_MODE_DEF_INPUT ) { gpio_input_enable(gpio_num);  } else { gpio_input_disable(gpio_num); }
    if (mode & GPIO_MODE_DEF_OUTPUT) { gpio_output_enable(gpio_num); } else { gpio_output_disable(gpio_num); }
    if (mode & GPIO_MODE_DEF_OD    ) { gpio_od_enable(gpio_num);     } else { gpio_od_disable(gpio_num); }

*/
static const struct {
  gpio_mode_t mode;
  const char * name;
} pin_modes[] = {
#define MODE(x) { GPIO_MODE_##x, #x }
  MODE(DISABLE),
  MODE(INPUT),
  MODE(OUTPUT),
  MODE(OUTPUT_OD),
  MODE(INPUT_OUTPUT_OD),
  MODE(INPUT_OUTPUT),
#undef MODE
};

static void pin_modes_list()
{
  for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
    LOG(". %u %s", pin_modes[i].mode, pin_modes[i].name);
  }
}

void pin_mode(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    LOG("tail='%s'", tail);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
      if (!strcasecmp(tail, pin_modes[i].name)) {
        LOG(". setting mode %u %s", pin_modes[i].mode, pin_modes[i].name);
        const auto mode = pin_modes[i].mode;
        if (ESP_OK != gpio_set_direction(gpio_num_t(pin), mode)) {
          LOG("ERROR: %s(pin=%u, mode=%u)", "gpio_set_direction", pin, mode);
        }
        return;
      }
    }
    unsigned v = command_args_to_ul(tail, nullptr);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
      if (pin_modes[i].mode == v) {
        LOG(". setting mode %u %s", pin_modes[i].mode, pin_modes[i].name);
        const auto mode = pin_modes[i].mode;
        if (ESP_OK != gpio_set_direction(gpio_num_t(pin), mode)) {
          LOG("ERROR: %s(pin=%u, mode=%u)", "gpio_set_direction", pin, mode);
        }
        return;
      }
    }
  }

  LOG("pin modes:");
  pin_modes_list();
}

/*
    esp_err_t gpio_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull);

      gpio_pull_mode_t

        GPIO_PULLUP_ONLY,               // Pad pull up
        GPIO_PULLDOWN_ONLY,             // Pad pull down
        GPIO_PULLUP_PULLDOWN,           // Pad pull up + pull down
        GPIO_FLOATING,                  // Pad floating
*/

static const struct {
  const char * name;
  gpio_pull_mode_t pull;
} pin_pulls[] = {
#define PULL(n, e) { n, GPIO_##e }
  PULL("UP"  , PULLUP_ONLY),
  PULL("DOWN", PULLDOWN_ONLY),
  PULL("BOTH", PULLUP_PULLDOWN),
  PULL("NONE", FLOATING),
#undef MODE
};

static void pin_pulls_list()
{
  for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
    LOG(". %u %s", pin_pulls[i].pull, pin_pulls[i].name);
  }
}

void pin_pull(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    LOG("tail='%s'", tail);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
      if (!strcasecmp(tail, pin_pulls[i].name)) {
        LOG(". setting pull %u %s", pin_pulls[i].pull, pin_pulls[i].name);
        const auto pull = pin_pulls[i].pull;
        if (ESP_OK != gpio_set_pull_mode(gpio_num_t(pin), pull)) {
          LOG("ERROR: %s(pin=%u, pull=%u)", "gpio_set_pull_mode", pin, pull);
        }
        return;
      }
    }
    unsigned v = command_args_to_ul(tail, nullptr);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
      if (pin_pulls[i].pull == v) {
        LOG(". setting pull %u %s", pin_pulls[i].pull, pin_pulls[i].name);
        const auto pull = pin_pulls[i].pull;
        if (ESP_OK != gpio_set_pull_mode(gpio_num_t(pin), pull)) {
          LOG("ERROR: %s(pin=%u, pull=%u)", "gpio_set_pull_mode", pin, pull);
        }
        return;
      }
    }
  }

  LOG("pin pulls:");
  pin_pulls_list();
}

/*
    esp_err_t gpio_input_enable(gpio_num_t gpio_num);
    esp_err_t gpio_output_enable(gpio_num_t gpio_num);
    esp_err_t gpio_output_disable(gpio_num_t gpio_num);
    esp_err_t gpio_od_disable(gpio_num_t gpio_num)
    esp_err_t gpio_od_enable(gpio_num_t gpio_num)

    'gpio_input_disable' exists but is inaccessible due to 'static':
    static esp_err_t gpio_input_disable(gpio_num_t gpio_num)
*/

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
    LOG("format: step_number"
        " step_ms"
        " sub_step_ms"
        " I_A"
        " PS_V_max"
        " cBat_max_mV"
        " cBat_min_mV"
        " aBat_max_mV"
        " aBat_min_mV"
        " cBat_T_max_C"
        " aBat_T_max_C");
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

void ProfileStep::apply(const char * args)
{
  const char * tail = args;

  if (!tail || !*tail) { return; }
  step_ms = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  sub_step_ms = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  I_A = command_args_to_i32(tail, &tail);

  if (!tail || !*tail) { return; }
  PS_V_max = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_max_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_min_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_max_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_min_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_T_max_C = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_T_max_C = command_args_to_u32(tail, &tail);
}

void ProfileStep::dump() const
{
  LOG(". this=%p"
      " step_ms=%u"
      " sub_step_ms=%u"
      " I_A=%i"
      " PS_V_max=%u"
      " cBat_max_mV=%u"
      " cBat_min_mV=%u"
      " aBat_max_mV=%u"
      " aBat_min_mV=%u"
      " cBat_T_max_C=%u"
      " aBat_T_max_C=%u",
      this,
      step_ms,
      sub_step_ms,
      I_A,
      PS_V_max,
      cBat_max_mV,
      cBat_min_mV,
      aBat_max_mV,
      aBat_min_mV,
      cBat_T_max_C,
      aBat_T_max_C);
}

static void report_config()
{
  Serial.print("device: '"); Serial.print(device_tag); Serial.println("'");
  spi_report_config();
}

void serial_write_buffer(
    const char * file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used)
{
  if (used < buffer_size) {
    Serial.write(buffer, used);
#if 1
    log(file, line, function,
        "buffer_size=%zu required=%zu", buffer_size, used + 1);
#endif
  } else {
    if (buffer_size >= 2) {
      buffer[buffer_size - 2] = '\n';
      buffer[buffer_size - 1] = '\0';
      Serial.write(buffer, buffer_size - 1);
    }
    log(file, line, function,
        "ERROR: buffer overflow (buffer_size=%zu required=%zu)",
        buffer_size, used + 1);
  }
}

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

char * skip_whitespace(char * p)
{
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  while (' ' == *p || '\t' == *p) {
    ++p;
    //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  }
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  return p;
}

//#define SPI_TRANS_TIME_STATS

const uint8_t spi_mr_mux_pins[SPI_MR_MUX_GPIO_COUNT] = {
  SPI_MR_MUX_0_GPIO,
  SPI_MR_MUX_1_GPIO,
  SPI_MR_MUX_2_GPIO,
  SPI_MR_MUX_3_GPIO,
};

// FIXME: Eliminate 'spi_tx_buffers'
byte spi_tx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
uint16_t spi_rx_bits[SPI_SENSOR_DEVICES_COUNT];
uint8_t spi_sensor_n_;

const spi_bus_config_t spi_bus_config = {
  .mosi_io_num = SPI_GPIO_MOSI,
  .miso_io_num = SPI_GPIO_MISO,
  .sclk_io_num = SPI_GPIO_SCLK,

  .quadwp_io_num = -1,
  .quadhd_io_num = -1,

  .data4_io_num = -1,
  .data5_io_num = -1,
  .data6_io_num = -1,
  .data7_io_num = -1,

#if 0
  // Output data IO default level when no transaction
  // Note: Looks like ESP32 'slave' does not support 'data_io_default_level'
  //.data_io_default_level = true,
  //.max_transfer_sz = SPI_BUFFER_SIZE,
  //.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#endif
};

/*
    Currently only one transaction is used.

    Multiple transactions submitted to driver can improve performance but will
    complicate cleanup in case of timeout.
*/
//#define SPI_QUEUE_SIZE SPI_SENSOR_DEVICES_COUNT
#define SPI_QUEUE_SIZE 1

/*
    SPI mode is combination of CPOL and CPHA:
      0: (0, 0)
      1: (0, 1)
      2: (1, 0)
      3: (1, 1)
*/
const spi_slave_interface_config_t spi_slave_interface_config = {
  .spics_io_num  = SPI_GPIO_CS,
  .flags         = SPI_SLAVE_NO_RETURN_RESULT, // suppresses results queue creation
  .queue_size    = SPI_QUEUE_SIZE,
  .mode          = 0,
  .post_setup_cb = spi_post_setup_cb,
  .post_trans_cb = spi_post_trans_cb
};


// 'MR' pins are 'OUTPUT' with default 'HIGH'/'inactive'

#define SPI_MR_ACTIvE   0
#define SPI_MR_INACTIvE 1

void spi_mr_pin_setup()
{
  LOG("");
  const auto spi_mr_pin = gpio_num_t(SPI_MR_GPIO);
  gpio_set_level(spi_mr_pin, 1);
  gpio_set_direction(spi_mr_pin, GPIO_MODE_INPUT_OUTPUT);
}

void spi_mr_mux_pins_setup()
{
  LOG("");
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, 0);
    gpio_set_direction(spi_mr_mux_pin, GPIO_MODE_INPUT_OUTPUT);
  }
}

void spi_mr_mux(unsigned spi_sensor_n)
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, (spi_sensor_n >> i) & 1);
  }
}

const uint8_t spi_pins[] = {
  SPI_GPIO_MOSI,
  SPI_GPIO_SCLK,
  SPI_GPIO_CS,
};

#if 0
// GPIO_DRIVE_CAP_0: ~5 mA
// GPIO_DRIVE_CAP_1: ~10 mA
// GPIO_DRIVE_CAP_2: ~20 mA -- default
// GPIO_DRIVE_CAP_3: ~40 mA
void spi_pins_setup()
{
  LOG("");
  // MISO pin (if used) is 'OUTPUT'.
  //pinMode(SPI_GPIO_MISO, INPUT_PULLUP);
  //pinMode(SPI_GPIO_MISO, OUTPUT);
  //digitalWrite(SPI_GPIO_MISO, HIGH);

  // MOSI, SCLK and CS pins are 'INPUT_PULLUP'
  for (unsigned i = 0; i < ARRAY_SIZE(spi_pins); ++i) {
    uint8_t spi_pin = spi_pins[i];
    pinMode(spi_pin, INPUT_PULLUP);
    //gpio_set_pull_mode(spi_pin, GPIO_PULLUP_ONLY);
    //gpio_set_drive_capability(spi_pin, GPIO_DRIVE_CAP_3);
  }
}
#endif


// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
unsigned spi_post_setup_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_setup_cb_us_;
#endif
void spi_post_setup_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_setup_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_setup_cb_us_ = esp_timer_get_time();
#endif

  spi_mr_mux(spi_sensor_n_);

  //spi_sensor_n_ %= SPI_SENSOR_DEVICES_COUNT;
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_ACTIvE);
}

//volatile
bool spi_transfer_finished_;

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
unsigned spi_post_trans_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_trans_cb_us_;
uint32_t spi_post_trans_cb_us_min_ = uint32_t(-1);
uint32_t spi_post_trans_cb_us_max_;
#endif
void spi_post_trans_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_trans_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_trans_cb_us_ = esp_timer_get_time();
  uint32_t trans_us = spi_post_trans_cb_us_ - spi_post_setup_cb_us_;
  if (spi_post_trans_cb_us_min_ > trans_us) {
    spi_post_trans_cb_us_min_ = trans_us;
  }
  if (spi_post_trans_cb_us_max_ < trans_us) {
    spi_post_trans_cb_us_max_ = trans_us;
  }
#endif

  //spi_sensor_n_ %= SPI_SENSOR_DEVICES_COUNT;
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  spi_rx_bits[spi_sensor_n_] = transaction->trans_len;

  spi_transfer_finished_ = true;
}


void spi_setup()
{
  LOG("");
  spi_mr_mux_pins_setup();
  spi_mr_pin_setup();
  LOG("");
}

#if 0
bool spi_init()
{
  //LOG("");
  //const auto r = spi_chip_init();
  const auto r = true;
  //LOG("");
  return r;
}

bool spi_down()
{
  //LOG("");

#if 0
  const auto spi_slave_disable_r = (ESP_OK == spi_slave_disable(SPI_DEVICE));
  if (!spi_slave_disable_r) {
    LOG("ERROR: '%s()' failure", "spi_slave_disable");
  }
  //LOG("");
#endif

#if 1
  const auto r = true;
#else
  const auto r = spi_chip_down();

  gpio_set_level(gpio_num_t(SPI_GPIO_MISO), 1);
  gpio_set_pull_mode(gpio_num_t(SPI_GPIO_MISO), GPIO_PULLUP_ONLY);
  gpio_set_direction(gpio_num_t(SPI_GPIO_MISO), GPIO_MODE_INPUT_OUTPUT);
#endif

  //LOG("");
  return r;
}
#endif


/*
    Warning: 'spi_slave_queue_trans()' does not make copy of
    'spi_slave_transaction_t'. Instead it keeps it's address and uses it to
    store number of actually transferred bits into 'trans_len' field. It means
    that memory occupied by 'spi_slave_transaction_t' must be preserved until
    the end of transmission which is notified by 'post_trans_cb' callback.
*/
static spi_slave_transaction_t spi_transaction_;

bool spi_transfer_start(uint8_t spi_sensor_n)
{
  spi_transfer_finished_ = false;

  spi_transaction_ = spi_slave_transaction_t {
    //.flags = 0,
    .length = SPI_DATA_SIZE * 8, // number of bits not bytes
    // trans_len -- used by driver to return actual number of received bits
    //.trans_len = 0,
    .tx_buffer = &spi_tx_buffers[spi_sensor_n][0],
    .rx_buffer = &spi_rx_buffers[spi_sensor_n][0],
    // void * user // User-defined variable.
    // Can be used to store 'spi_sensor_n' for 'spi_post_setup_cb' and 'spi_post_trans_cb'.
    //.user = NULL,
  };
  if (ESP_OK != spi_slave_queue_trans(SPI_DEVICE, &spi_transaction_, 0)) {
    LOG("ERROR: %s() failure", "spi_slave_queue_trans");
    return false;
  }
  return true;
}


#define SPI_RX_BITS_MIN (SPI_DATA_SIZE * 8)

void spi_rx_bits_reset()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    spi_rx_bits[i] = 0;
  }
}

bool spi_rx_bits_valid()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    if (spi_rx_bits[i] < SPI_RX_BITS_MIN) {
      //LOG(" . spi_rx_bits[%u]=%u SPI_RX_BITS_MIN=%u", i, spi_rx_bits[i], SPI_RX_BITS_MIN);
      return false;
    }
  }
  return true;
}


SPI_Device spiDevice;


const char * SPI_Device::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "running",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void SPI_Device::state(State new_state)
{
  const auto old_state_name = to_string(state_);
  state_ = new_state;
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
}

void SPI_Device::on_state_init(uint32_t now_ms)
{
  (void) now_ms;
  state( (spi_chip_init()) ? State::idle : State::init_failed);
}

// Can be used to implement delay before init retry
void SPI_Device::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
}

void SPI_Device::on_state_idle(uint32_t now_ms)
{
  (void) now_ms;
  if (run_) {
    run_ = false;

    //scope_sync_on(true);
    transfer_start_time_ms_ = now_ms;

    constexpr uint32_t spi_transfer_all_timeout_ms = 100;
    timeout_time_ms_ = now_ms + spi_transfer_all_timeout_ms;

    spi_rx_bits_reset();
    spi_sensor_n_ = 0;
    sensors_data_valid_ = false;

    memset(spi_rx_buffers, 'R', sizeof(spi_rx_buffers));
    //memset(spi_tx_buffers, 'T', sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x00, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x0F, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xFF, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xC3, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0xF0, sizeof(spi_tx_buffers));
    //memset(spi_tx_buffers, 0x81, sizeof(spi_tx_buffers));
    memset(spi_tx_buffers, 0x99, sizeof(spi_tx_buffers));

    if (spi_transfer_start(spi_sensor_n_)) {
      state(State::running);
    } else {
      //scope_sync_on(false);
      state(State::idle);
    }
  }
}

/*
    'spi_transfer_finished_' marks end of single transfer.

    TODO: Implement some state for multiple transfers to integrate them with
    'Test' running.
*/
void SPI_Device::on_state_running(uint32_t now_ms)
{
  if (spi_transfer_finished_) {
    if (++spi_sensor_n_ < SPI_SENSOR_DEVICES_COUNT) {
      if (!spi_transfer_start(spi_sensor_n_)) {
        //scope_sync_on(false);
        state(State::idle);
      }
    } else {
      sensors_data_valid_ = spi_rx_bits_valid();
      //LOG("sensors_data_valid_=%i", sensors_data_valid_);
      // FIXME: check CRCs
      transfer_finish_time_ms_ = now_ms;
      //scope_sync_on(false);
      state(State::idle);
    }
  } else
  if (timeout_time_ms_ <= now_ms) {
    LOG("WARNING: %s %s", "spi transfer", "timeout");

    // make sure MR pin is inactive
    gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

    //scope_sync_on(false);
    state(State::idle);
  }
}

void SPI_Device::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(init_failed)
    CASE_CALL(idle)
    CASE_CALL(running)
#undef CASE_CALL
    default:
      //LOG("ERROR: Unexpected state=%u", state_);
      LOG("ERROR: Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

void spi_run(const char * args)
{
  (void) args;
  spiDevice.run();
}

void spi_report_config()
{
  LOG(". %s=%u", "SPI_GPIO_MISO", SPI_GPIO_MISO);
  LOG(". %s=%u", "SPI_GPIO_MOSI", SPI_GPIO_MOSI);
  LOG(". %s=%u", "SPI_GPIO_SCLK", SPI_GPIO_SCLK);
  LOG(". %s=%u", "SPI_GPIO_CS", SPI_GPIO_CS);
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = spi_mr_mux_pins[i];
    LOG(". SPI_MR_MUX_%u_GPIO=%u", i, spi_mr_mux_pin);
  }
  LOG(". %s=%u", "SPI_MR_GPIO", SPI_MR_GPIO);
  LOG(". %s=%u", "SPI_BUFFER_SIZE", SPI_BUFFER_SIZE);
  LOG(". %s=%u", "SPI_DATA_SIZE", SPI_DATA_SIZE);
  LOG(". %s=%u", "SPI_RX_BITS_MIN", SPI_RX_BITS_MIN);
  LOG(". %s=%u", "sizeof(spi_rx_buffers)", sizeof(spi_rx_buffers));
}

void spi_report_stats()
{
  LOG(". spi_post_setup_cb_count_=%u spi_post_trans_cb_count_=%u",
      spi_post_setup_cb_count_, spi_post_trans_cb_count_);
#if defined SPI_TRANS_TIME_STATS
  LOG(". spi_post_trans_cb_us_min_=%u spi_post_trans_cb_us_max_=%u",
      spi_post_trans_cb_us_min_, spi_post_trans_cb_us_max_);
#endif

  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    hex_dump(&spi_rx_buffers[i][0], SPI_DATA_SIZE);
  }
}

void spi_stats(const char * args)
{
  (void) args;
  spi_report_stats();
  SensorDevices_Data_report_all();
}

bool spi_chip_active_ = false;

bool spi_chip_init()
{
  LOG("");
  if (spi_chip_active_) { return true; }

  const auto r = (ESP_OK == spi_slave_initialize(SPI_DEVICE,
      &spi_bus_config, &spi_slave_interface_config, SPI_DMA_DISABLED));
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_initialize");
  } else {
    spi_chip_active_ = true;
  }
  return r;
}

bool spi_chip_down()
{
  if (!spi_chip_active_) { return true; }

  const auto r = (ESP_OK == spi_slave_free(SPI_DEVICE));
  if (!r) {
    LOG("ERROR: '%s()' failure", "spi_slave_free");
  } else {
    spi_chip_active_ = false;
  }
  return r;
}

void analog_report_one(const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals.all[i];
  Serial.print(" ");
  //Serial.print(analogSignal);
  //Serial.print("/");
  Serial.print(analogSignal * 0.001, 3);
}

void analog_report_all(const SensorDeviceData_t & deviceData)
{
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    analog_report_one(deviceData, i);
  }
  Serial.println("");
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
void SensorDevice_Data_report_one(unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);

#if 0
  //LOG(". sizeof(SensorDeviceData_t)=%zu LGT8F_SPI_DATA_SIZE==%zu", sizeof(SensorDeviceData_t), LGT8F_SPI_DATA_SIZE);
  uint8_t checksum = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.checksum));
  LOG(". checksum=0x%02X", checksum);
#endif
  hex_dump( (const uint8_t *)&deviceData, sizeof(deviceData));

  Serial.print(". . ");
  Serial.print(deviceData.vcc);
  Serial.print("/");
  Serial.print(deviceData.vcc * 0.00125, 4);
  Serial.print(" |");
  analog_report_all(deviceData);
}

void SensorDevices_Data_report_all()
{
  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    SensorDevice_Data_report_one(i);
  }
}


/*
  uint8_t checksum = calcCRC8(
    (const uint8_t *) &deviceData,
    LGT8F_SPI_DATA_SIZE - sizeof(deviceData.checksum));
  Serial.print(". checksum="); Serial.println(checksum, HEX);
*/

namespace rpi {

static size_t analog_report_one(
    char * buffer, size_t buffer_size,
    const SensorDeviceData_t & deviceData, unsigned i)
{
  const auto analogSignal = deviceData.analogSignals.all[i];
  return safe_snprintf(buffer, buffer_size, " %4u", analogSignal);
}

static size_t analog_report_all(
    char * buffer, size_t buffer_size,
    const SensorDeviceData_t & deviceData)
{
  size_t used = 0;
  for (unsigned i = 0; i < ANALOG_SIGNALS_COUNT; ++i) {
    if (buffer_size > used) {
      const size_t left = buffer_size - used;
      used += ::rpi::analog_report_one(&buffer[used], left, deviceData, i);
    }
  }
  return used;
}

// byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
static size_t SensorDevice_Data_report_one(
    char * buffer, size_t buffer_size,
    unsigned i)
{
  const auto & deviceData = *(SensorDeviceData_t*)(&spi_rx_buffers[i][0]);

#if 1
  size_t used = safe_snprintf(buffer, buffer_size, "  %6u", deviceData.vcc * 125);
#else
  size_t used = safe_snprintf(buffer, buffer_size, "  %4u", deviceData.vcc);
#endif
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += ::rpi::analog_report_all(&buffer[used], left, deviceData);
  }
  return used;
}

/*
    now_ms
    spiDevice.transfer_start_time_ms_
    spiDevice.transfer_finish_time_ms_
*/
#define SensorDevices_Data_STRLEN ( \
  CONSTSTRLEN("/TEST/SPI") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + CONSTSTRLEN("  123456789") \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 1234") * ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSOR_DEVICES_COUNT \
  + CONSTSTRLEN("  FF\n") \
)

static void SensorDevices_Data_report_all(
    uint32_t now_ms,
    uint32_t test_start_time_ms)
{
  static char buffer[SensorDevices_Data_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

#if 0
  const auto us1 = esp_timer_get_time(); // uint64_t
  const auto availableForWrite1 = cn_uartConfig.availableForWrite();
#endif

  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/SPI");
#if 0
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, "  %10" PRIu64, us1);
  }
#endif

  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        now_ms - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        spiDevice.transfer_start_time_ms_ - test_start_time_ms);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, " %9u",
        spiDevice.transfer_finish_time_ms_ - test_start_time_ms);
  }

  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    if (buffer_size > used) {
      const size_t left = buffer_size - used;
      used += ::rpi::SensorDevice_Data_report_one(&buffer[used], left, i);
    }
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, "  XX\n");
  }

  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);

#if 0
  const auto availableForWrite2 = cn_uartConfig.availableForWrite();
  const auto us2 = esp_timer_get_time(); // uint64_t
  LOG("availableForWrite1=%u - availableForWrite2=%u =%i",
      availableForWrite1, availableForWrite2,
      int(availableForWrite1) - int(availableForWrite2));
  LOG("us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, us2, us1, us2 - us1);
#endif
}

} // namespace rpi

Test test;

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

void Test::dump() const
{
  hex_dump( (const uint8_t*)this, sizeof(*this));
  LOG("step=%u sub_step=%u test_start_time_ms=%u sub_step_end_time_ms=%u"
      " spi_timeout_time_ms=%u run=%i abort=%i state=%u/%s",
      step_, sub_step_, test_start_time_ms_, sub_step_end_time_ms_,
      spi_timeout_time_ms_, run_, abort_, state_, to_string(state_));
}

void test_dump(const char * args)
{
  test.dump();
}

void Test::change_PS_EL_configs(const ProfileStep * profileStep)
{
  if (PS_V_max != profileStep->PS_V_max) {
    LOG("'PS_V_max' change (%u -> %u)", PS_V_max, profileStep->PS_V_max);
    psDevice.off();
    elDevice.off();
    if (profileStep->I_A > 0) {
      psDevice.v(profileStep->PS_V_max);
      psDevice.c(+ profileStep->I_A);
      elDevice.c(0);
      psDevice.on();
    } else if (profileStep->I_A < 0) {
      psDevice.v(profileStep->PS_V_max);
      psDevice.c(0);
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  } else
  if (sign(I_A) != sign(profileStep->I_A)) {
    LOG("'I_A' direction change (%i -> %i)", I_A, profileStep->I_A);
    if (profileStep->I_A > 0) {
      elDevice.c(0);
      psDevice.c(+ profileStep->I_A);
      psDevice.on();
    } else
    if (profileStep->I_A < 0) {
      psDevice.c(0);
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  } else
  if (I_A != profileStep->I_A) {
    LOG("'I_A' value change (%i -> %i)", I_A, profileStep->I_A);
    if (profileStep->I_A > 0) {
      psDevice.c(+ profileStep->I_A);
      psDevice.on();
    } else
    if (profileStep->I_A < 0) {
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  }
  PS_V_max = profileStep->PS_V_max;
  I_A = profileStep->I_A;
}

void Test::deactivate_PS_EL()
{
  LOG("deactivating EL");
  psDevice.off();
  elDevice.off();
  PS_V_max = 0;
  I_A = 0;
}

void Test::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(idle)
    CASE_CALL(step_init)
    CASE_CALL(sub_step_init)
    CASE_CALL(sub_step_wait)
    CASE_CALL(sub_step_measure)
    CASE_CALL(spi_wait)
    CASE_CALL(sub_step_next)
    CASE_CALL(step_next)
    CASE_CALL(abort)
    CASE_CALL(cleanup)
#undef CASE_CALL
    default:
      LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "step_init",
    "sub_step_init",
    "sub_step_wait",
    "sub_step_measure",
    "spi_wait",
    "sub_step_next",
    "step_next",
    "abort",
    "cleanup",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

#define TEST_STATE_STRLEN CONSTSTRLEN("/TEST/STATE sub_step_measure\n")

void test_state(const char * args)
{
  (void) args;

  char buffer[TEST_STATE_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STATE %s\n", Test::to_string(test.state_));

  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);
}

void Test::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    abort_ = false;

    if (!profile.steps_count_) {
      LOG("ERROR: profile is empty");
      state(State::idle);
    } else {

      // --- 'test_init' ---

      // timings for all steps and all measurement sub steps will be based on
      // 'test_start' time
      test_start_time_ms_   = now_ms;
      step_end_time_ms_     = now_ms;
//    sub_step_end_time_ms_ = now_ms;

      // --- 'step_0' ---
      step_ = 0;

      state(State::step_init);
    }
  }
}

#define TEST_STEP_START_STRLEN CONSTSTRLEN("/TEST/STEP 99\n")

void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---

  char buffer[TEST_STEP_START_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STEP %2u\n", step_);
  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);

  const ProfileStep * profileStep = &profileSteps_[step_];

  /*
    'step_end_time_ms_' shall be relative to 'step [planned] start time'.

    Here 'step_end_time_ms_' contains previous step 'planned' end time. It also
    acts as current step 'planned' start time and can differ from 'now_ms' due
    to slow CPU.
  */
  const auto step_start_time_ms = step_end_time_ms_;
#if 0
  LOG("DEBUG: step_start_time_ms=%u '%s' now_ms=%u",
      step_start_time_ms,
      (step_start_time_ms == now_ms) ? "==" : "!=",
      now_ms);
#endif
  sub_step_end_time_ms_ = step_start_time_ms;

  step_end_time_ms_ += profileStep->step_ms;
#if 0
  LOG(". step_end_time_ms_=%u", step_end_time_ms_);
#endif

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_step_ = 0;

  state(State::sub_step_init);
}

/*
    if !step_ms skip waiting and jump directly to 'sub_step_measure'.

    '!step_ms' can be useful to do measurements at the beginning of the
    test immediately after changing PS and EL configs.

    if !sub_step_ms then next '*_sub_step_end' will be quickly executed
    just after few 'on_loop'.

    '!sub_step_ms' can be useful to do multiple repeated measurementsat
    the beginning of the test immediately after changing PS and EL configs.
*/

void Test::on_state_sub_step_init(uint32_t now_ms)
{
  // --- 'sub_step_init' ---

  const ProfileStep * profileStep = &profileSteps_[step_];
  /*
    'sub_step_end_time_ms_' shall be relative to 'sub step [planned] start
    time'.

    Here 'sub_step_end_time_ms_' contains previous sub step 'planned' end time.
    It also acts as current sub step 'planned' start time and can differ from
    'now_ms' due to slow CPU.
  */
  const auto sub_step_start_time_ms = sub_step_end_time_ms_;
#if 0
  LOG("sub_step_start_time_ms=%u now_ms=%u", sub_step_start_time_ms, now_ms);
#endif
  sub_step_end_time_ms_ = sub_step_start_time_ms + profileStep->sub_step_ms;
#if 0
  LOG(". sub_step_end_time_ms_=%u", sub_step_end_time_ms_);
#endif
  state(State::sub_step_wait);
}

// Long running state. Check for user's abort request.

// 'sub_step_wait'
void Test::on_state_sub_step_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (sub_step_end_time_ms_ <= now_ms) {
    // --- 'sub_step_end' ---
    LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);
    state(State::sub_step_measure);
  }
}

// 'sub_step_measure'
void Test::on_state_sub_step_measure(uint32_t now_ms)
{
  // --- 'sub_step_measure' ---

  // --- 'spi_init' ---
  // FIXME: Use correct 'spi_timeout_ms'
  constexpr uint32_t spi_timeout_ms = 50;
  //constexpr uint32_t spi_timeout_ms = 5;
  spi_timeout_time_ms_ = now_ms + spi_timeout_ms;
#if 0
  LOG(". spi_timeout_time_ms_=%u", spi_timeout_time_ms_);
#endif
  spiDevice.run();
  state(State::spi_wait);
}

// Long running state. Check for user's abort request.

// 'spi_wait'
void Test::on_state_spi_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (spiDevice.isTransferFinished()) {
    //LOG("'transferFinished'");

    // --- 'spi transfer end' ---

    if (!spiDevice.areSensorsDataValid()) {
      LOG("ERROR: received sensors' data are invalid");
      state(State::abort);
    } else {
#if 0
      uint64_t us_old = 0;
      uint32_t ms_old = 0;
      for (unsigned i = 0; i < 100; ++i) {
        const auto us = esp_timer_get_time(); // uint64_t
        const auto ms = millis();
        const auto availableForWrite = cn_uartConfig.availableForWrite();
        LOG("%9" PRIu64 " %6u %3u %9" PRIu64 " %6u %4u", us, ms, i, us-us_old, ms-ms_old, availableForWrite);
        us_old = us;
        ms_old = ms;
      }
#endif
      // report sensors' data
      //SensorDevices_Data_report_all();
      rpi::SensorDevices_Data_report_all(now_ms, test_start_time_ms_);

      // start next sub step
      state(State::sub_step_next);

    }
  } else
  if (spi_timeout_time_ms_ <= now_ms) {
    LOG("ERROR: sensors' data SPI transfer timeout");
    state(State::abort);
  }
}

void Test::on_state_sub_step_next(uint32_t now_ms)
{
  // 'sub_step_next'

  const ProfileStep * profileStep = &profileSteps_[step_];
  ++sub_step_;
  LOG(". sub_step_=%u", sub_step_);
  if (now_ms < step_end_time_ms_) {
//  sub_step_end_time_ms_ += profileStep->sub_step_ms;
    state(State::sub_step_init);
  } else {
    LOG("--- step_=%u finished -- start next step (change PS and EL configs) ...", step_);
    state(State::step_next);
  }
}

void Test::on_state_step_next(uint32_t now_ms)
{
  // --- 'step_next' ---
  ++step_;
  LOG(". step_=%u", step_);
  if (step_ < profile.steps_count_) {
    state(State::step_init);
  } else {
    state(State::cleanup);
  }
}

// abort test execution.
// Reached on user's request or on critical condition detection.

void Test::on_state_abort(uint32_t now_ms)
{
  abort_ = false;
  LOG("test execution aborted");
  state(State::cleanup);
}

void Test::on_state_cleanup(uint32_t now_ms)
{
  // --- 'cleanup' ---
  deactivate_PS_EL();
  LOG("--- test finished ---");
  Serial.print("/TEST/FINISHED\n");
  state(State::idle);
}

const UartConfig cn_uartConfig = {
  .tag =  "CN-u" MAKE_STRING(CN_UART_NUM),
  .hs = &Serial,
  .baud = CN_UART_BAUD,
  .n = 0,
  .rxd_gpio = -1, // '-1' == 'do not change'
  .txd_gpio = -1, // '-1' == 'do not change'
  .rxBufferSize = 0,
  .txBufferSize = 0x800,
//.txBufferSize = 0x1000,
};

static const UartConfig ps_uartConfig = {
  .tag =  "PS-u" MAKE_STRING(PS_UART_NUM),
  .hs = &Serial1,
  .baud = PS_UART_BAUD,
  .n = 1,
  .rxd_gpio = PS_UART_RX_GPIO,
  .txd_gpio = PS_UART_TX_GPIO,
  .rxBufferSize = 0,
  .txBufferSize = 0,
};

static const UartConfig el_uartConfig = {
  .tag = "EL-u" MAKE_STRING(EL_UART_NUM),
  .hs = &Serial2,
  .baud = EL_UART_BAUD,
  .n = 2,
  .rxd_gpio = EL_UART_RX_GPIO,
  .txd_gpio = EL_UART_TX_GPIO,
  .rxBufferSize = 0,
  .txBufferSize = 0,
};

const UartConfig * const xx_uartConfigs[] = {
  &cn_uartConfig,
  &ps_uartConfig,
  &el_uartConfig,
};

void xx_uartConfigs_init() {
  for (unsigned i = 0; i < ARRAY_SIZE(xx_uartConfigs); ++i) {
    xx_uartConfigs[i]->init();
  }
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

void EL_Device::clear_errors() const
{
  const char * cmd = "*CLS";
  write_cmd(cmd);
}

void EL_Device::function() const
{
  const char * cmd = "INPut:FUNCtion?";
  write_cmd(cmd);
}

void EL_Device::function_xx(unsigned v)
{
  function_ = v;

  char cmd[sizeof("INPut:FUNCtion XX")];
  snprintf(cmd, sizeof(cmd), "INPut:FUNCtion %s", el_function_to_string(v));
  write_cmd(cmd);

  if (cmd_verify_) { function(); }
}

void EL_Device::function_CC()
{
  function_xx(0);
}

void EL_Device::function_CV()
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
void EL_Device::cc_range() const
{
  const char * cmd = "STATic:CC:RANGe?";
  write_cmd(cmd);
}

void EL_Device::cc_range(unsigned v)
{
  cc_range_ = v;

  char cmd[sizeof("STATic:CC:RANGe X")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:RANGe %u", v);
  write_cmd(cmd);

  if (cmd_verify_) { cc_range(); }
}

void EL_Device::cc_xx_level(unsigned range) const
{
  char cmd[sizeof("STATic:CC:xxxx:LEVel?")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel?", el_range_to_string(range));
  write_cmd(cmd);
}

void EL_Device::cc_level() const
{
  cc_xx_level(cc_range_);
}

/*
  u2_lf STATic:CC:HIGH:LEVel 1.12345 OK
  u2_lf STATic:CC:HIGH:LEVel 12.1234 OK
  u2_lf STATic:CC:HIGH:LEVel 123.123 OK
  u2_lf STATic:CC:HIGH:LEVel 1234.12 NG
*/
void EL_Device::cc_xx_level(unsigned range, double v) const
{
  char cmd[sizeof("STATic:CC:xxxx:LEVel 123.123")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel %.5f", el_range_to_string(range), v);
  write_cmd(cmd);

  if (cmd_verify_) { cc_xx_level(range); }
}

void EL_Device::cc_level(double v) const
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
void EL_Device::cv_range() const
{
  const char * cmd = "STATic:CV:RANGe?";
  write_cmd(cmd);
}

void EL_Device::cv_range(unsigned v)
{
  cv_range_ = v;

  char cmd[sizeof("STATic:CV:RANGe X")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:RANGe %u", v);
  write_cmd(cmd);

  if (cmd_verify_) { cv_range(); }
}

void EL_Device::cv_xx_level(unsigned range) const
{
  char cmd[sizeof("STATic:CV:xxxx:LEVel?")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel?", el_range_to_string(range));
  write_cmd(cmd);
}

void EL_Device::cv_level() const
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
void EL_Device::cv_xx_level(unsigned range, double v) const
{
  char cmd[sizeof("STATic:CV:xxxx:LEVel 123.123")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel %.5f", el_range_to_string(range), v);
  write_cmd(cmd);

  if (cmd_verify_) { cv_xx_level(range); }
}

void EL_Device::cv_level(double v) const
{
  cv_xx_level(cv_range_, v);
}

void EL_Device::get_input() const
{
  const char * cmd = "INPut:STATe?";
  write_cmd(cmd);
}

void EL_Device::on()
{
  if (on_) { return; }

  const char * cmd = "INPut:STATe ON";
  write_cmd(cmd);

  on_ = true;

  if (cmd_verify_) { get_input(); }
}

void EL_Device::off()
{
  if (!on_) { return; }

  const char * cmd = "INPut:STATe OFF";
  write_cmd(cmd);

  on_ = false;

  if (cmd_verify_) { get_input(); }
}

void EL_Device::c() const
{
  cc_range();
  cc_level();
}

void EL_Device::c(double v)
{
  cc_range(0); // high range
  cc_level(v);
}

void PS_Device::v() const
{
  const char * cmd = "V1?";
  write_cmd(cmd);
}

void PS_Device::c() const
{
  const char * cmd = "I1?";
  write_cmd(cmd);
}

void PS_Device::ovp() const
{
  const char * cmd = "OVP1?";
  write_cmd(cmd);
}

void PS_Device::ocp() const
{
  const char * cmd = "OCP1?";
  write_cmd(cmd);
}

void PS_Device::output() const
{
  const char * cmd = "OP1?";
  write_cmd(cmd);
}

/*
  "I1"
  "V1"
  "OCP1"
  "OVP1"
  ------
  "XXX1"
*/
void PS_Device::x(const char * s, double v) const
{
  char cmd[sizeof("XXX1 12.123")];
  snprintf(cmd, sizeof(cmd), "%s %.3f", s, v);
  write_cmd(cmd);

  if (cmd_verify_) {
    snprintf(cmd, sizeof(cmd), "%s?", s);
    write_cmd(cmd);
  }
}

void PS_Device::v(double v) const
{
  x("V1", v);
}

void PS_Device::c(double v) const
{
  x("I1", v);
}

void PS_Device::ovp(double v) const
{
  x("OVP1", v);
}

void PS_Device::ocp(double v) const
{
  x("OCP1", v);
}

void PS_Device::on()
{
  if (on_) { return; }

  const char * cmd = "OP1 1";
  write_cmd(cmd);

  on_ = true;

  if (cmd_verify_) { output(); }
}

void PS_Device::off()
{
  if (!on_) { return; }

  const char * cmd = "OP1 0";
  write_cmd(cmd);

  on_ = false;

  if (cmd_verify_) { output(); }
}

static CN_Device cnDevice = {
  /*.uartConfig =*/ &cn_uartConfig,
};

// 'TENMA 72-2710 V6.6 SN:31428462'
// 'THURLBY THANDAR, QPX1200SP, 620839, 4.31-4.08'

PS_Device psDevice {
  /*.uartConfig =*/ &ps_uartConfig,
  /*.expected_id =*/
  //"TENMA 72-2710 V6.6 SN:31428462",
  "THURLBY THANDAR, QPX1200SP, 620839, 4.31-4.08",
};

void ps_id(const char * args)
{
  (void) args;
  psDevice.id();
}

void ps_v(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.v(v);
  }
  psDevice.v();
}

void ps_c(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.c(v);
  }
  psDevice.c();
}

void ps_ovp(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.ovp(v);
  }
  psDevice.ovp();
}

void ps_ocp(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.ocp(v);
  }
  psDevice.ocp();
}

void ps_output(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "on")) {
      psDevice.on();
    } else
    if (!strcasecmp(args, "off")) {
      psDevice.off();
    } else {
      LOG("output must be ON or OFF");
    }
  }
  psDevice.output();
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
      elDevice.on();
    } else
    if (!strcasecmp(args, "off")) {
      elDevice.off();
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

void loop()
{
  //LOG("sp=%p", cpu_sp_get());

  static uint32_t loop_count = 0;

  uint32_t now_ms = millis();
  const auto now_s = now_ms / 1000;

#if 1
  static uint32_t report_config_s = 0;
  if (now_s - report_config_s >= 5*60) {
    report_config();
    report_config_s = now_s;
    LOG("uxTaskGetStackHighWaterMark()=%u", uxTaskGetStackHighWaterMark(NULL));
  }
#endif

  static uint32_t report_count_s = 0;
  if (now_s - report_count_s >= 5) {
    Serial.print(device_tag);
    Serial.print(" now_ms="); Serial.print(now_ms);
    Serial.print(" loop_count="); Serial.println(loop_count);
    //spi_report_stats();

    report_count_s = now_s;
  }

  uartDevices_on_loop(now_ms);
  spiDevice.on_loop(now_ms);
  test.on_loop(now_ms);

  ++loop_count;
}

void setup()
{
  xx_uartConfigs_init();
  //scope_sync_setup();
  spi_setup();
  report_config();
}
