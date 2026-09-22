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

#include "esp_err.h" // ESP_xxx, ESP_ERROR_CHECK()

#include "esp_log.h" // ESP_LOG*()

#include "driver/gpio.h"

extern "C" {
esp_err_t gpio_output_enable(gpio_num_t gpio_num);
}

#include "driver/i2c_master.h"

#include "driver/spi_slave.h"
#include "esp_private/spi_slave_internal.h" // spi_slave_queue_reset()

#include "driver/uart.h"

//#define COLLECT_MIN_MAX

#define CN_UART_NUM             0
//#define CN_UART_BAUD            115200
//#define CN_UART_BAUD            3000000
#define CN_UART_BAUD            2000000
//#define CN_UART_BAUD            1000000

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

/*
    There is little dirty trick.

    Normally 'used <= size'. However 'used > size' is permitted and means
    [logical] 'overflow' with excessive data lost.
*/

namespace nc {
namespace mk {

struct Buffer
{
  char * data_;
  size_t size_;
  size_t used_;

  Buffer(char * data, size_t size) noexcept
    : data_(data)
    , size_(size)
    , used_(0)
  {
  }

  size_t used() const noexcept { return std::min(used_, size_); }
  size_t need() const noexcept { return used_; }

  __attribute__ ((format (printf, 1, 2)));
  size_t printf(const char * format, ...) noexcept;
  size_t vprintf(const char * format, va_list argsList) noexcept;

  size_t put_char(char c) noexcept;
  size_t put(const char * data, size_t size) noexcept;
  size_t put_s(const char * s) noexcept;

  inline
  Buffer & operator<<(char c) noexcept
  {
    put_char(c);
    return *this;
  }

  inline
  Buffer & operator<<(const char * s) noexcept
  {
    put_s(s);
    return *this;
  }

  size_t put(uint64_t v) noexcept;
  size_t put(int64_t v) noexcept;
  size_t put(uint32_t v) noexcept;
  size_t put(int32_t v) noexcept;

  inline size_t put(uint16_t v) noexcept { return put(uint32_t(v)); }
  inline size_t put(int16_t v) noexcept { return put(int32_t(v)); }
  inline size_t put(uint8_t v) noexcept { return put(uint32_t(v)); }
  inline size_t put(int8_t v) noexcept { return put(int32_t(v)); }

  inline size_t put(unsigned v) noexcept { return put(uint32_t(v)); }
  inline size_t put(signed v) noexcept { return put(int32_t(v)); }

#if 1
  template <typename T>
  inline Buffer & operator<<(T v) noexcept { put(v); return *this; }
#else
  inline Buffer & operator<<(uint64_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint32_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint16_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(int16_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint8_t v) noexcept { put(v); return *this; }

  inline Buffer & operator<<(unsigned v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(signed v) noexcept { put(v); return *this; }
#endif
};

} // namespace mk
} // namespace nc

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

namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint32_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint32_t number,
    uint8_t base,
    bool upperCase);

} // namespace cvt
} // namespace mk
} // namespace nc

namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint64_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint64_t number,
    uint8_t base,
    bool upperCase);

} // namespace cvt
} // namespace mk
} // namespace nc

struct Runnable
{
  virtual void on_loop(uint32_t now_ms)
  {
    LOG("");
  }
};

namespace nc {
namespace stats {

using counter_t = uint64_t;

} // namespace stats
} // namespace nc

namespace nc {
namespace stats {

struct OkNg
{
  counter_t counter_[2];

  static constexpr const char * const tags[] = {
    "ok",
    "ng",
  };

  // 0 OK
  // 1 NG
  inline
  size_t index_(bool ok) const noexcept
  {
    return !ok;
  }

  inline
  const counter_t & counter(bool ok) const noexcept
  {
    return counter_[index_(ok)];
  }

  inline
  counter_t & counter(bool ok) noexcept
  {
    return counter_[index_(ok)];
  }

  inline
  const counter_t & ok() const noexcept
  {
    return counter(true);
  }

  inline
  counter_t & ok() noexcept
  {
    return counter(true);
  }

  inline
  const counter_t & ng() const noexcept
  {
    return counter(false);
  }

  inline
  counter_t & ng() noexcept
  {
    return counter(false);
  }

  inline
  const counter_t & operator[](bool ok) const noexcept
  {
    return counter(ok);
  }

  inline
  counter_t & operator[](bool ok) noexcept
  {
    return counter(ok);
  }

  inline
  void inc(bool ok) noexcept
  {
    ++counter(ok);
  }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace stats
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::stats::OkNg & okNg) noexcept
{
  return okNg.to_buffer(b);
}

double command_args_to_double(const char * args, const char ** tail);

int32_t command_args_to_i32(const char * args, const char ** tail);

uint32_t command_args_to_u32(const char * args, const char ** tail);

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

void command_echo(const char * args);

void el_id(const char * args);
void el_clear_errors(const char * args);
void el_function(const char * args);
void el_input(const char * args);
void el_cc_level(const char * args);
void el_cv_level(const char * args);
void el_cc_range(const char * args);
void el_cv_range(const char * args);

void command_idn(const char * args);

void ps_id(const char * args);
void ps_clear_alerts(const char * args);
void ps_v(const char * args);
void ps_c(const char * args);
void ps_ovp(const char * args);
void ps_ocp(const char * args);
void ps_output(const char * args);

void stats(const char * args);

void ux_baud(unsigned n, const char * args);

void u1_baud(const char * args);
void u2_baud(const char * args);

void ux_hex(unsigned n, const char * args);

void ux_cr(unsigned n, const char * args);

void ux_crlf(unsigned n, const char * args);

void ux_lf(unsigned n, const char * args);

void ux_lfcr(unsigned n, const char * args);

void u1_hex(const char * args);
void u2_hex(const char * args);
void u1_cr(const char * args);
void u2_cr(const char * args);
void u1_crlf(const char * args);
void u2_crlf(const char * args);
void u1_lf(const char * args);
void u2_lf(const char * args);
void u1_lfcr(const char * args);
void u2_lfcr(const char * args);

#define ucLOG(format, args...) LOG("%s| " format, tag, ##args)
struct UartConfig {
  const char * const tag;
  HardwareSerial * hs;
  uint32_t baud;

  size_t rxBufferSize;
  size_t txBufferSize;

  uint8_t n;

  int8_t rxd_gpio;
  int8_t txd_gpio;

  uint8_t rx_fifo_irq_bytes;

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
    if (rx_fifo_irq_bytes) { hs->setRxFIFOFull(rx_fifo_irq_bytes); }
#if 0
    if (0 == n) {
      /*
        #define XON  0x11
        #define XOFF 0x13

        rx_thresh_xon  < 128
        rx_thresh_xoff < 128

        esp_err_t uart_set_sw_flow_ctrl(
            uart_port_t uart_num,
            bool enable,
            uint8_t rx_thresh_xon,
            uint8_t rx_thresh_xoff)
      */
      if (ESP_OK != uart_set_sw_flow_ctrl(uart_port_t(0), true, 20, 80)) {
        LOG("ERROR: '%s()' %s", "uart_set_sw_flow_ctrl", "failure");
      } else {
        LOG("INFO: '%s()' %s", "uart_set_sw_flow_ctrl", "success");
      }
    }
#endif
    // FIXME: Move the following to something like uartConfig_report_config()':
    report();
  }

  void report() const
  {
    ucLOG(". Rx GPIO=%i", rxd_gpio);
    ucLOG(". Tx GPIO=%i", txd_gpio);
    ucLOG(". baud=%u", baud);
    if (rxBufferSize) { ucLOG(". rxBufferSize=%u", rxBufferSize); }
    if (txBufferSize) { ucLOG(". txBufferSize=%u", txBufferSize); }
    if (rx_fifo_irq_bytes) { ucLOG(". rx_fifo_irq_bytes=%u", rx_fifo_irq_bytes); }
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
      cnLOG("There is no command '%*s'", (int)len, s);
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

extern CN_Device cnDevice;

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

  void clear_alerts() const;

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

void i2c_adc_config(const char * args);

void i2c_adc_read(const char * args);

void i2c_adc_read_repeat(const char * args);

void i2c_adc_single(const char * args);

void i2c_adc_write(const char * args);

void i2c_probe(const char * args);

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html

namespace nc {
namespace i2c {

struct Bus
{
  const i2c_master_bus_config_t & i2c_master_bus_config_;
  i2c_master_bus_handle_t i2c_master_bus_handle_;

  bool init_done_ = false;

  bool init();
  void down();

  bool probe(uint16_t address);
};

extern Bus bus;

struct Device
{
  const Bus & bus_;
  const i2c_device_config_t & i2c_device_config_;
  i2c_master_dev_handle_t i2c_master_dev_handle_;

  bool init_done_ = false;

  bool init();
  void down();

  bool read(uint8_t * read_buffer, size_t read_size);
  bool write(const uint8_t * write_buffer, size_t write_size);
  bool write_read(
      const uint8_t * write_buffer, size_t write_size,
      uint8_t * read_buffer, size_t read_size);
};

// Note: 'device' instance is specific for 'Adc' instance.
// FIXME: Move 'device' into 'Adc'
extern Device device;


namespace adc_regs {

struct Config
{
  uint16_t COMP_QUE:2;  // 1..0
    /*
        Comparator queue and disable (ADS1114 and ADS1115 only)

        These bits perform two functions. When set to 11, the comparator is
        disabled and the ALERT/RDY pin is set to a high-impedance state. When
        set to any other value, the ALERT/RDY pin and the comparator function
        are enabled, and the set value determines the number of successive
        conversions exceeding the upper or lower threshold required before
        asserting the ALERT/RDY pin.

        00b : Assert after one conversion
        01b : Assert after two conversions
        10b : Assert after four conversions
        11b : Disable comparator and set ALERT/RDY pin to high-impedance (default)
    */
  uint16_t COMP_LAT:1;  // 2
    /*
        Latching comparator (ADS1114 and ADS1115 only)

        Controls whether the ALERT/RDY pin latches after being asserted or
        clears after conversions are within the margin of the upper and lower
        threshold values.

        0b : Nonlatching comparator. The ALERT/RDY pin does not latch when
        asserted (default).

        1b : Latching comparator. The asserted ALERT/RDY pin remains latched
        until conversion data are read by the controller or an appropriate
        SMBus alert response is sent by the controller. The device responds
        with an address, and is the lowest address currently asserting the
        ALERT/RDY bus line.
    */
  uint16_t COMP_POL:1;  // 3
    /*
        Comparator polarity. Controls polarity of the ALERT/RDY pin.

        0b : Active low (default)
        1b : Active high
    */
  uint16_t COMP_MODE:1; // 4
    /*
        Comparator mode

        0b : Traditional comparator (default)
        1b : Window comparator
    */
  uint16_t DR:3;        // 7..5
    /*
        Data rate

        000b : 8SPS
        001b : 16SPS
        010b : 32SPS
        011b : 64SPS
        100b : 128SPS (default)
        101b : 250SPS
        110b : 475SPS
        111b : 860SPS
    */
  uint16_t MODE:1;      // 8
    /*
        Device operating mode

        0b : Continuous-conversion mode
        1b : Single-shot mode or power-down state (default)
    */
  uint16_t PGA:3;       // 11..9
    /*
        Programmable gain amplifier configuration

        These bits set the FSR of the programmable gain amplifier.

        000b : FSR = ±6.144V(1)
        001b : FSR = ±4.096V(1)
        010b : FSR = ±2.048V (default)
        011b : FSR = ±1.024V
        100b : FSR = ±0.512V
        101b : FSR = ±0.256V
        110b : FSR = ±0.256V
        111b : FSR = ±0.256V
    */
  uint16_t MUX:3;       // 14..12
    /*
        Input multiplexer configuration (ADS1115 only)

        These bits configure the input multiplexer.

        000b : AINP = AIN0 and AINN = AIN1 (default)
        001b : AINP = AIN0 and AINN = AIN3
        010b : AINP = AIN1 and AINN = AIN3
        011b : AINP = AIN2 and AINN = AIN3
        100b : AINP = AIN0 and AINN = GND
        101b : AINP = AIN1 and AINN = GND
        110b : AINP = AIN2 and AINN = GND
        111b : AINP = AIN3 and AINN = GND

        Adafruit_ADS1X15.h:

        CONFIG_MUX_DIFF_0_1 (0x0000) // Differential P = AIN0, N = AIN1 (default)
        CONFIG_MUX_DIFF_0_3 (0x1000) // Differential P = AIN0, N = AIN3
        CONFIG_MUX_DIFF_1_3 (0x2000) // Differential P = AIN1, N = AIN3
        CONFIG_MUX_DIFF_2_3 (0x3000) // Differential P = AIN2, N = AIN3

        CONFIG_MUX_SINGLE_0 (0x4000) // Single-ended AIN0
        CONFIG_MUX_SINGLE_1 (0x5000) // Single-ended AIN1
        CONFIG_MUX_SINGLE_2 (0x6000) // Single-ended AIN2
        CONFIG_MUX_SINGLE_3 (0x7000) // Single-ended AIN3
    */
  uint16_t OS:1;        // 15
    /*
        Operational status or single-shot conversion start

        This bit determines operational status of the device. OS can only be
        written when in power-down state and has no effect when a conversion is
        ongoing.

        When writing:
        0b : No effect
        1b : Start a single conversion (when in power-down state)

        When reading:
        0b : Device is currently performing a conversion.
        1b : Device is not currently performing a conversion.
    */
};

void dump(Config c);

} // namespace adc_regs

struct Adc
{
#if 0
    constexpr uint8_t Conversion = 0; // P[1:0] = 00b, reset = 0000h
    constexpr uint8_t Config     = 1; // P[1:0] = 01b, reset = 8583h
    constexpr uint8_t Lo_thresh  = 2; // P[1:0] = 10b, reset = 8000h
    constexpr uint8_t Hi_thresh  = 3; // P[1:0] = 11b, reset = 7FFFh
#else
  enum class Register
  {
    Conversion = 0, // P[1:0] = 00b, reset = 0000h
    Config     = 1, // P[1:0] = 01b, reset = 8583h
    Lo_thresh  = 2, // P[1:0] = 10b, reset = 8000h
    Hi_thresh  = 3, // P[1:0] = 11b, reset = 7FFFh
  };
#endif

  Device & device_;

  bool init_done_ = false;

  bool init();
  void down();

  bool read(Register rn, uint16_t & v);
  bool read_repeat(uint16_t & v);
  bool write(Register rn, uint16_t v);

  bool start_single_conversion(unsigned channel);
};

extern const i2c_master_bus_config_t i2c_master_bus_config;
extern const i2c_device_config_t i2c_device_config;

extern Adc adc;

} // namespace i2c
} // namespace nc

void i2c_probe(const char * args);
void i2c_adc_read(const char * args);
void i2c_adc_read_repeat(const char * args);
void i2c_adc_write(const char * args);

void i2c_adc_report(const char * args);

void i2c_adc_run(const char * args);

void i2c_adc_stats(const char * args);

struct I_Meter
{
  struct Status
  {
    bool transfer_done;

    inline void reset() { transfer_done = false; }

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } status;

  uint16_t value; // conversion_result;
  void update_value();

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meter::Status & status) noexcept
{
  return status.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meter & iMeter) noexcept
{
  return iMeter.to_buffer(b);
}

//#define iMeters_state_verbose

#define I_METERS_COUNT 2

struct I_Meters
  : Runnable
{
  struct Stats {
    ::nc::stats::OkNg start;
    ::nc::stats::OkNg poll;
    ::nc::stats::OkNg read;

    static constexpr const char * const tags[] = {
      "start",
      "poll",
      "read",
    };

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } stats;

  uint32_t start_time_ms_;
  uint32_t end_time_ms_;
  static constexpr uint32_t single_conversion_timeout_ms = 5;
  uint32_t timeout_time_ms_;

  I_Meter iMeters_[I_METERS_COUNT];

  uint8_t channel_;

  bool start();
  bool poll(bool & ready);
  bool read(uint16_t & v);

  enum class State : uint8_t {
    init,
    init_failed,
    idle,
    transfer_init,
    transfer_wait,
    transfer_timeout,
    next_channel,
    run_finished,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_transfer_init(uint32_t now_ms);
  void on_state_transfer_wait(uint32_t now_ms);
  void on_state_transfer_timeout(uint32_t now_ms);
  void on_state_next_channel(uint32_t now_ms);
  void on_state_run_finished(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  bool run_ = false;
  inline void run() { run_ = true; }
  bool run_finished_ = false;
  inline bool run_finished() /*const*/ { return run_finished_; }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

extern I_Meters iMeters;

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meters::Stats& stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meters & iMeters) noexcept
{
  return iMeters.to_buffer(b);
}

namespace nc {
namespace i2c {
namespace stats {

void report();

} // namespace stats
} // namespace i2c
} // namespace nc

namespace nc {
namespace lgt {

using VCC = uint16_t;

using Voltage = int16_t;
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const Voltage voltage);

static constexpr const unsigned voltage_pins_COUNT = 4;
using Voltages = Voltage[voltage_pins_COUNT];
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const ::nc::lgt::Voltages & voltages);

using Temperature = uint16_t;
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const Temperature temperature);

static constexpr const unsigned temperature_pins_COUNT = 4;
static constexpr const unsigned temperature_groups_COUNT = 2;
using Temperatures = Temperature[temperature_pins_COUNT][temperature_groups_COUNT];
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const ::nc::lgt::Temperatures & temperatures);

static constexpr const unsigned ALL_ANALOG_SIGNALS_COUNT =
    voltage_pins_COUNT + temperature_pins_COUNT * temperature_groups_COUNT;
using AllAnalogSignals = int16_t[ALL_ANALOG_SIGNALS_COUNT];

union AnalogSignals
{
  struct
  {
    Voltages voltages;
    Temperatures temperatures;
  };
  AllAnalogSignals all;

  ::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b) const noexcept;
};

struct SensorData
{
  uint8_t format; // data format version number, 1 for now
  uint8_t device_id;
  AnalogSignals analogSignals;
  VCC vcc;
  uint8_t crc;

  ::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace lgt
} // namespace nc

/*
    sizeof(SensorData) on
      LGT8F  21
      ESP32  22 one extra byte is added after crc
*/
#define LGT8F_SPI_DATA_SIZE OFFSET_OF_END(::nc::lgt::SensorData, crc)

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Voltages & voltages)
{
  return ::nc::lgt::to_buffer(b, voltages);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Temperatures & temperatures)
{
  return ::nc::lgt::to_buffer(b, temperatures);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::AnalogSignals & analogSignals)
{
  return analogSignals.to_buffer(b);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::SensorData & sensorData)
{
  return sensorData.to_buffer(b);
}

void stats_disable(const char * args);

void stats_enable(const char * args);

namespace nc {
namespace loop {

using time_t = uint32_t;

time_t now() { return ::millis(); }

struct Stats
  : Runnable
{
  uint32_t loop_count_ = 0;

  static constexpr uint32_t report_stats_period_ms = 5000;
  uint32_t report_stats_time_ms = 0;

  static constexpr uint32_t report_config_period_ms = 5*60*1000;
  uint32_t report_config_time_ms = 0;

  bool disabled_ = false;
  void disable() { disabled_ = true; }
  void enable() { disabled_ = false; }

  void on_loop(uint32_t now_ms) override;

  void report(uint32_t now_ms) const;
};

extern Stats stats_;
  
} // namespace loop
} // namespace nc

void monitor_I(const char * args);

void monitor_period(const char * args);

void monitor_raw(const char * args);

void monitor_start(const char * args);

void monitor_stop(const char * args);

void monitor_T(const char * args);

void monitor_V(const char * args);

//#define nc_Monitor_state_verbose

namespace nc {

struct Monitor
  : Runnable
{
  bool active_ = false;
  bool start_ = false;
  bool stop_ = false;

  struct Measure
  {
    static constexpr uint32_t timeout_ms = 10;
    uint32_t timeout_time_ms_;
  } measure_;

  // FIXME: 'pause' -> 'Measure::Period'
  struct Pause
  {
    uint32_t period_ms_ = 5000;
    uint32_t end_time_ms_;
  } pause_;

  enum class State : uint8_t {
    idle,
    start,
    stop,
    measure_init,
    measure_wait,
    measure_done,
    measure_report,
    measure_timeout,
    pause_init,
    pause_wait,
    pause_done,
  };
  State state_ = State::idle;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_idle(uint32_t now_ms);
  void on_state_start(uint32_t now_ms);
  void on_state_stop(uint32_t now_ms);
  void on_state_measure_init(uint32_t now_ms);
  void on_state_measure_wait(uint32_t now_ms);
  void on_state_measure_done(uint32_t now_ms);
  void on_state_measure_report(uint32_t now_ms);
  void on_state_measure_timeout(uint32_t now_ms);
  void on_state_pause_init(uint32_t now_ms);
  void on_state_pause_wait(uint32_t now_ms);
  void on_state_pause_done(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  void start() { start_ = true; }
  void stop() { stop_ = true; }

  struct
  {
    bool raw_ = false;
    bool I_ = false;
    bool V_ = false;
    bool T_ = false;
  } disable_;

  void report() const;
  void report_VT(unsigned spi_sensor_n) const;
};

extern Monitor monitor_;

} // namespace nc

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

void profile_step(const char * args);

void profile_steps_dump(const char * args);

void profile_steps(const char * args);

void report_config();

void serial_write_buffer(
    const char *file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used);

#define SERIAL_WRITE_BUFFER(buffer, buffer_size, used) \
    serial_write_buffer( \
        __FILE__, __LINE__, __PRETTY_FUNCTION__, \
        buffer, buffer_size, used);

static constexpr size_t echo_buffer_size = 4096;

static constexpr size_t spi_data_buffer_size = 4096;

static constexpr size_t spi_raw_data_buffer_size = 4096;

static constexpr size_t spi_stats_buffer_size = 4096;

static constexpr size_t uart_stats_buffer_size = 4096;

static constexpr size_t uart_test_buffer_size = 4096;

namespace nc {

typedef union shared_buffers_u
{
  char echo_buffer[echo_buffer_size];
  char spi_data_buffer[spi_data_buffer_size];
  char spi_raw_data_buffer[spi_raw_data_buffer_size];
  char spi_stats_buffer[spi_stats_buffer_size];
  char uart_stats_buffer[uart_stats_buffer_size];
  char uart_test_buffer[uart_test_buffer_size];
} shared_buffers_t;

extern shared_buffers_t shared_buffers;

} // namespace nc

const char * skip_whitespace(const char * p);
char * skip_whitespace(char * p);

#define SPI_BUFFER_SIZE_MAX 64
#define SPI_BUFFER_SIZE SPI_BUFFER_SIZE_MAX
//#define SPI_BUFFER_SIZE 128
#define SPI_DATA_SIZE 29

#define SPI_DEVICE SPI2_HOST

#define SPI_GPIO_MISO 12
#define SPI_GPIO_MOSI 13
#define SPI_GPIO_SCLK 14
#define SPI_GPIO_CS   15

// 'Master Request' (MR) is used to request SPI transfer from one of many LGT8F
// devices selected by 'MR_MUX'
#define SPI_MR_GPIO 4

#define SPI_SENSORS_COUNT 8

#define SPI_RX_BITS_MIN (SPI_DATA_SIZE * 8)

namespace nc {
namespace cli {

void spi_config(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_data(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_data_raw(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_dump(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_run(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_stats(const char * args);

} // namespace cli
} // namespace nc

extern const spi_bus_config_t spi_bus_config;

extern bool spi_chip_active_;

bool spi_chip_down();

bool spi_chip_init();

namespace nc {
namespace spi {
namespace hw {

struct Chip
{
  bool active_ = false;
  bool init();
  void down();
};

} // namespace hw
} // namespace spi
} // namespace nc

extern const uint8_t spi_pins[];

// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
void spi_post_setup_cb(spi_slave_transaction_t * transaction);

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
void spi_post_trans_cb(spi_slave_transaction_t * transaction);

extern unsigned spi_post_trans_cb_count_;

extern const spi_slave_interface_config_t spi_slave_interface_config;

// state of 'single' transfer
extern bool spi_transfer_active_;

bool spi_transfer_start(uint8_t spi_sensor_n);

void spi_mr_mux(unsigned spi_sensor_n);

// 'Master Request' (MR) multiplexer (MUX) is used to select one of many LGT8F
// devices
#define SPI_MR_MUX_GPIO_COUNT 4
#define SPI_MR_MUX_0_GPIO 32
#define SPI_MR_MUX_1_GPIO 33
#define SPI_MR_MUX_2_GPIO 19
#define SPI_MR_MUX_3_GPIO 18

extern const uint8_t spi_mr_mux_pins[SPI_MR_MUX_GPIO_COUNT];

void spi_mr_mux_pins_setup();

// 'MR' pins are 'OUTPUT' with default 'HIGH'/'inactive'

#define SPI_MR_ACTIvE   0
#define SPI_MR_INACTIvE 1

void spi_mr_pin_setup();

void spi_pins_setup();

namespace nc {
namespace spi {

void report_build_config();

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

struct Sensor
{
  struct Stats
  {
    ::nc::stats::OkNg transfer;
    ::nc::stats::OkNg rx_bits;
    ::nc::stats::OkNg crc;

    static constexpr const char * const tags[] = {
      "transfer",
      "rx_bits",
      "crc",
    };

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } stats;

  struct Raw
  {
    uint8_t tx_buffer[SPI_BUFFER_SIZE];
    union
    {
      uint8_t rx_buffer[SPI_BUFFER_SIZE];
      ::nc::lgt::SensorData data;
    };

    struct Status
    {
      bool transfer_done;

      uint16_t rx_bits;
      bool crc_valid_;

      inline bool rx_bits_valid() const { return rx_bits >= SPI_RX_BITS_MIN; }
      inline bool is_crc_valid() const { return crc_valid_; }

      ::nc::mk::Buffer &
      to_buffer(::nc::mk::Buffer & b) const noexcept;
    } status;

    uint8_t crc() const;
    bool is_crc_valid_impl() const;
    void update_crc_valid();

    void reset();

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } raw;

  ::nc::lgt::SensorData data;
  void update_data();

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace spi
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Stats & stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Raw::Status & status) noexcept
{
  return status.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Raw & raw) noexcept
{
  return raw.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor & sensor) noexcept
{
  return sensor.to_buffer(b);
}

//#define spi_Sensors_state_verbose

namespace nc {
namespace spi {

struct Sensors
  : Runnable
{
  struct Stats
  {
    ::nc::stats::counter_t post_setup_cb_count;
    ::nc::stats::counter_t post_trans_cb_count;

    static constexpr const char * const tags[] = {
      "setup_cb",
      "trans_cb",
    };

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } stats;

  ::nc::mk::Buffer &
  stats_to_buffer(::nc::mk::Buffer & b) const noexcept;

  uint32_t transfer_start_time_ms_;
  uint32_t transfer_end_time_ms_;
  static constexpr uint32_t single_transfer_timeout_ms = 5;
  uint32_t timeout_time_ms_;

  Sensor sensors_[SPI_SENSORS_COUNT];

  struct HW
  {
    ::nc::spi::hw::Chip chip_;
    //static
    spi_slave_transaction_t transaction_;
    bool active_;
    uint16_t rx_bits() const { return transaction_.trans_len; }
  } hw;
  bool hw_transfer_start();

  uint8_t sensor_n_;

  bool offline_[SPI_SENSORS_COUNT];
  void offline_config(uint32_t v);
  void offline_config_dump() const;

  void skip_offline();
  inline bool offline(unsigned sensor_n) const { return offline_[sensor_n]; }

  enum class State : uint8_t {
    init,
    init_failed, // can be used to retry init after some delay
    idle,
    transfer_init,
    transfer_wait,
    transfer_timeout,
    next_sensor,
    run_finished,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_transfer_init(uint32_t now_ms);
  void on_state_transfer_wait(uint32_t now_ms);
  void on_state_transfer_timeout(uint32_t now_ms);
  void on_state_next_sensor(uint32_t now_ms);
  void on_state_run_finished(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  bool run_ = false;
  inline void run() { run_ = true; }
  bool run_finished_ = false;
  inline bool run_finished() const { return run_finished_; }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

extern Sensors sensors;

} // namespace spi
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensors::Stats & stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensors & sensors) noexcept
{
  return sensors.to_buffer(b);
}

void spi_setup();

void analog_report_one(const ::nc::lgt::SensorData & sensorData, unsigned i);
void analog_report_all(const ::nc::lgt::SensorData & sensorData);
void SensorDevice_Data_report_one(unsigned i);
void spi_report_data();

namespace nc {
namespace spi {
namespace stats {

void report();

} // namespace stats
} // namespace spi
} // namespace nc

void test_abort(const char * args);

void test_dump(const char * args);

void test_run(const char * args);

void test_state(const char * args);

struct Test
  : Runnable
{
  uint32_t test_start_time_ms_;

  struct Step
  {
    unsigned count_;
    uint32_t start_time_planned_ms_;
    uint32_t start_time_actual_ms_;
    uint32_t end_time_planned_ms_;
  } step_;
  static constexpr unsigned step_count_max_ = 100;

  struct Sub
  {
    Step step_;
  } sub_;
  static constexpr unsigned sub_step_count_max_ = 10000;

  struct Measure
  {
    uint32_t start_time_actual_ms_;
    uint32_t end_time_actual_ms_;

    // FIXME: Use correct 'Measure::timeout_ms'
    static constexpr uint32_t timeout_ms = 10;
    uint32_t timeout_time_ms_;
  } measure_;

  int32_t I_A = 0;
  uint32_t PS_V_max = 0;

  bool run_;
  bool abort_;

  enum class State : uint8_t {
    idle,
//  init,                   // start_time_ms_ = sub_step_end_time_ms_ = now_ms;
//  step_0,                 // step_ = 0;
    step_init,              // 1 set PS/EL configs
//       sub_step_0,        // sub_step_ = 0;
         sub_step_init,     // 2 'sub_step_end_time_ms_ += measure_period_ms_;'
         sub_step_wait,     // 3 wait for end of sub step (sub_step_end_time_ms_)
           measure_init,
//           spi_init,      //   start SPI transfer to get measurements
//           i2c_adc_init,  //   start i2c ADC measurements
           measure_wait,    //   wait for end of SPI transfer and i2c ADC measurements
         sub_step_next,     // 5 '++sub_step_ < substeps_count_ ? sub_step_init : step_next'
    step_next,              // 6 '++step_ < steps_count_ ? step_init : cleanup'
    abort,                  // 7 report and cleanup (deactivate PS and EL)
    cleanup,                // 8 deactivate PS and EL
  };
  State state_ = State::idle;
  static const char * to_string(State state);

  void state(State new_state);

  void change_PS_EL_configs(const ProfileStep * profileStep);
  void deactivate_PS_EL();

  void on_loop(uint32_t now_ms) override;

  void on_state_idle(uint32_t now_ms);
  void on_state_step_init(uint32_t now_ms);
  void on_state_sub_step_init(uint32_t now_ms);
  void on_state_sub_step_wait(uint32_t now_ms);
  void on_state_measure_init(uint32_t now_ms);
  void on_state_measure_wait(uint32_t now_ms);
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

  void report_step_data(
      uint32_t now_ms,
      uint32_t test_start_time_ms) const;

};

extern Test test;

namespace nc {
namespace cli {

void uart_stats(const char * args);

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void uart_test(const char * args);

} // namespace cli
} // namespace nc

void uart_stats_report();

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

namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint32_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint32_t number,
    uint8_t base,
    bool upperCase)
{
  size_t buffer_index = 0;
  if (!number)
  {
    if (buffer_index < buffer_size) { buffer[buffer_index] = '0'; }
    ++buffer_index;
  }
  else
  {
    uint8_t digits[sizeof(number)*8];
    size_t digitIndex = 0;
    while (number)
    {
      digits[digitIndex++] = number % base;
      number /= base;
    }
    while (digitIndex)
    {
      uint8_t digit = digits[--digitIndex];
      digit += (digit < 10) ? '0' : upperCase ? ('A' - 10) : ('a' - 10);
      if (buffer_index < buffer_size) { buffer[buffer_index] = digit;}
      ++buffer_index;
    }
  }
  return buffer_index;
}

} // namespace cvt
} // namespace mk
} // namespace nc

namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint64_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint64_t number,
    uint8_t base,
    bool upperCase)
{
  size_t buffer_index = 0;
  if (!number)
  {
    if (buffer_index < buffer_size) { buffer[buffer_index] = '0'; }
    ++buffer_index;
  }
  else
  {
    uint8_t digits[sizeof(number)*8];
    size_t digitIndex = 0;
    while (number)
    {
      digits[digitIndex++] = number % base;
      number /= base;
    }
    while (digitIndex)
    {
      uint8_t digit = digits[--digitIndex];
      digit += (digit < 10) ? '0' : upperCase ? ('A' - 10) : ('a' - 10);
      if (buffer_index < buffer_size) { buffer[buffer_index] = digit;}
      ++buffer_index;
    }
  }
  return buffer_index;
}

} // namespace cvt
} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::printf(const char* format, ...) noexcept
{
  va_list argsList;
  va_start(argsList, format);
  size_t used = vprintf(format, argsList);
  va_end(argsList);
  return used;
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put(const char * data, size_t size) noexcept
{
  for (size_t i = 0; i < size; ++i) {
    put_char(data[i]);
  }
  return size;
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put_char(char c) noexcept
{
  if (size_ > used_) {
    char * data = &data_[used_];
    size_t free = size_ - used_;
    if (free > 1) {
      data[0] = c;
      data[1] = '\0';
    } else {
      data[0] = '\0';
    }
  }
  size_t used = 1;
  used_ += used;
  return used;
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put(int32_t v) noexcept
{
  uint32_t u;
  if (v < 0) {
    u = -v;
    put_char('-');
  } else {
    u = v;
  }
  return put(u);
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put(int64_t v) noexcept
{
  uint64_t u;
  if (v < 0) {
    u = -v;
    put_char('-');
  } else {
    u = v;
  }
  return put(u);
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put_s(const char * s) noexcept
{
  if (!s) { s = "<nullptr>"; }
  size_t used = 0;
  for (char c = *s; c; ++s, c = *s) {
    put_char(c);
    ++used;
  }
  return used;
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put(uint32_t v) noexcept
{
  char buffer[sizeof(v)*8];
  const size_t size = ::nc::mk::cvt::uint32_to_ascii(buffer, sizeof(buffer), v, 10, false);
  return put(buffer, size);
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::put(uint64_t v) noexcept
{
  char buffer[sizeof(v)*8];
  const size_t size = ::nc::mk::cvt::uint64_to_ascii(buffer, sizeof(buffer), v, 10, false);
  return put(buffer, size);
}

} // namespace mk
} // namespace nc

namespace nc {
namespace mk {

size_t Buffer::vprintf(const char * format, va_list argsList) noexcept
{
  char * data;
  size_t free;
  char buffer[1];
  if (size_ > used_) {
    data = &data_[used_];
    free = size_ - used_;
  } else {
    data = buffer;
    free = sizeof(buffer);
  }
  size_t used = safe_vsnprintf(data, free, format, argsList);
  used_ += used;
  return used;
}

} // namespace mk
} // namespace nc

const char CMD_IDENTIFY[] = "*IDN?";

double command_args_to_double(const char * args, const char ** tail)
{
  double v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!args || !*args) {
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

int32_t command_args_to_i32(const char * args, const char ** tail)
{
  unsigned v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!args || !*args) {
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

uint32_t command_args_to_u32(const char * args, const char ** tail)
{
  //LOG("arg=%p/'%s'", args, args ? args : "");
  unsigned v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!args || !*args) {
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

using namespace ::nc::cli;

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
  COMMAND("echo", command_echo),
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

  COMMAND(spi_config),
  COMMAND(spi_data),
  COMMAND(spi_data_raw),
  COMMAND(spi_dump),
  COMMAND(spi_run),
  COMMAND(spi_stats),

  COMMAND(ps_id),
  COMMAND(ps_clear_alerts),
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

  COMMAND(i2c_probe),
  COMMAND(i2c_adc_read),
  COMMAND(i2c_adc_read_repeat),
  COMMAND(i2c_adc_write),
  COMMAND(i2c_adc_config),
  COMMAND(i2c_adc_single),

  COMMAND(i2c_adc_run),
  COMMAND(i2c_adc_report),
  COMMAND(i2c_adc_stats),

  COMMAND(stats),
  COMMAND(stats_disable),
  COMMAND(stats_enable),

  COMMAND(monitor_start),
  COMMAND(monitor_stop),
  COMMAND(monitor_period),
  COMMAND(monitor_raw),
  COMMAND(monitor_I),
  COMMAND(monitor_V),
  COMMAND(monitor_T),

  COMMAND(uart_stats),
  COMMAND(uart_test),
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

void command_echo(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.echo_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/ECHO '%s'\n", args);

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}

void command_idn(const char * args)
{
  (void) args;
  LOG("NubaCell BT ESP32 v.0.0 sn:1234567890");
}

void stats(const char * args)
{
  (void) args;
  uart_stats_report();
  ::nc::i2c::stats::report();
  ::nc::spi::stats::report();
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

void i2c_adc_config(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  ::nc::i2c::adc_regs::dump( *(::nc::i2c::adc_regs::Config*) &v);
}

void i2c_adc_read(const char * args)
{
  unsigned register_number = command_args_to_u32(args, nullptr);
  LOG("register_number=0x%X/%u", register_number, register_number);
  if (register_number >= 4) {
    LOG("ERROR: register_number must be < 4");
    return;
  }
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register(register_number), v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}

void i2c_adc_read_repeat(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read_repeat(v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}

void i2c_adc_single(const char * args)
{
  unsigned channel = command_args_to_u32(args, nullptr);
  LOG("channel=%u", channel);
  if (channel >= 4) {
    LOG("ERROR: channel must be < 4");
    return;
  }

  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint64_t us1 = esp_timer_get_time();
  {
    const auto r = ::nc::i2c::adc.start_single_conversion(channel);
    //LOG("r=%i", r);
    if (!r) { return; }
  }
  uint64_t us2 = esp_timer_get_time();
  unsigned wait_count = 0;
  bool converting;
  do {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read_repeat(v);
    //LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
    if (!r) { return; }
    ++wait_count;
    if (wait_count > 100) {
      LOG("wait_count limit exceeded");
      break;
    }
    const auto c = *(::nc::i2c::adc_regs::Config*) &v;
    converting = !c.OS;
  } while (converting);
  uint64_t us3 = esp_timer_get_time();
#if 0
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }
#endif
  uint64_t us4;
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
    us4 = esp_timer_get_time();
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i '%s'", r, v, v, int16_t(v), "Conversion");
  }
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us2, us1, us2 - us1);
  LOG("%s us3=%" PRIu64 " - us2=%" PRIu64 " =%" PRIu64, "write_read", us3, us2, us3 - us2);
  LOG("%s us4=%" PRIu64 " - us3=%" PRIu64 " =%" PRIu64, "write_read", us4, us3, us4 - us3);
  LOG("%s us4=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us4, us1, us4 - us1);
  LOG("wait_count=%u", wait_count);
}

void i2c_adc_write(const char * args)
{
  const char * tail = NULL;
  unsigned register_number = command_args_to_u32(args, &tail);
  LOG("register_number=0x%X/%u", register_number, register_number);
  if (register_number >= 4) {
    LOG("ERROR: register_number must be < 4");
    return;
  }

  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  if (tail && *tail) {
    unsigned v = command_args_to_u32(tail, nullptr);
    const auto r = ::nc::i2c::adc.write(::nc::i2c::Adc::Register(register_number), v);
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }

  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register(register_number), v);
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }
}

void i2c_probe(const char * args)
{
  unsigned device = command_args_to_u32(args, nullptr);
  LOG("device=0x%X/%u", device, device);
  if (!::nc::i2c::bus.init_done_) {
    if (!::nc::i2c::bus.init()) { return; }
  }
  const auto probe_r = ::nc::i2c::bus.probe(device);
  LOG("probe_r=%i", probe_r);
}

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html

//#define I2C_TIME_STATS

constexpr uint32_t I2C_TIMEOUT_MS = 2;

namespace nc {
namespace i2c {

const i2c_master_bus_config_t i2c_master_bus_config = {
  .i2c_port = 0, // I2C_MASTER_NUM,
  .sda_io_num = gpio_num_t(21),
  .scl_io_num = gpio_num_t(22),
  .clk_source = I2C_CLK_SRC_DEFAULT,
  .glitch_ignore_cnt = 7,
  //.trans_queue_depth = 1, // must '>0' for 'async' mode
  //.flags = { .enable_internal_pullup = true, },
  .flags = { .enable_internal_pullup = false, },
};

Bus bus = {
  .i2c_master_bus_config_ = i2c_master_bus_config,
};

// address must be 0x49 (default is 0x48)
#define i2c_adc_address 0x49

// 400kHz is actually 384kHz
#define i2c_adc_frequency 400000
//#define i2c_adc_frequency 650000
// at 675kHz i2c adc frequently fails

const i2c_device_config_t i2c_device_config = {
  .dev_addr_length = I2C_ADDR_BIT_LEN_7,
  .device_address = i2c_adc_address,
  .scl_speed_hz = i2c_adc_frequency,
};

Device device = {
  .bus_ = bus,
  .i2c_device_config_ = i2c_device_config,
};

/*
    esp_err_t i2c_new_master_bus(
        const i2c_master_bus_config_t * bus_config,
        i2c_master_bus_handle_t * ret_bus_handle)

    const char * esp_err_to_name(esp_err_t code)
*/
bool Bus::init()
{
  if (init_done_) { return true; }

  const auto e = i2c_new_master_bus(
      &i2c_master_bus_config_, &i2c_master_bus_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_new_master_bus", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = true;
  }
  return r;
}

/*
    esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus_handle)
*/
void Bus::down()
{
  if (!init_done_) { return; }

  const auto e = i2c_del_master_bus(i2c_master_bus_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_del_master_bus", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = false;
  }
}

/*
    esp_err_t i2c_master_probe(
        i2c_master_bus_handle_t bus_handle,
        uint16_t address,
        int xfer_timeout_ms)
*/
bool Bus::probe(uint16_t address)
{
  const auto e = i2c_master_probe(
      i2c_master_bus_handle_, address, I2C_TIMEOUT_MS);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_probe", "failure", e, esp_err_to_name(e));
  }
  return r;
}


/*
    esp_err_t i2c_master_bus_add_device(
        i2c_master_bus_handle_t bus_handle,
        const i2c_device_config_t * dev_config,
        i2c_master_dev_handle_t * ret_handle)
*/
bool Device::init()
{
  if (init_done_) { return true; }

  const auto e = i2c_master_bus_add_device(
      bus_.i2c_master_bus_handle_,
      &i2c_device_config, &i2c_master_dev_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_bus_add_device", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = true;
  }
  return r;
}

/*
    esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t handle)
*/
void Device::down()
{
  if (!init_done_) { return; }

  const auto e = i2c_master_bus_rm_device(i2c_master_dev_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_bus_rm_device", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = false;
  }
}

/*
    esp_err_t i2c_master_receive(
        i2c_master_dev_handle_t i2c_dev,
        uint8_t * read_buffer, size_t read_size,
        int xfer_timeout_ms)
*/
bool Device::read(uint8_t * read_buffer, size_t read_size)
{
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_receive(
      i2c_master_dev_handle_,
      read_buffer, read_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "i2c_master_receive", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_receive", "failure", e, esp_err_to_name(e));
  } else {
    //hex_dump(read_buffer, read_size);
  }
  return r;
}

/*
    esp_err_t i2c_master_transmit(
        i2c_master_dev_handle_t i2c_dev,
        const uint8_t * write_buffer, size_t write_size,
        int xfer_timeout_ms)
*/
bool Device::write(const uint8_t * write_buffer, size_t write_size)
{
  //hex_dump(write_buffer, write_size);
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_transmit(
      i2c_master_dev_handle_,
      write_buffer, write_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "i2c_master_transmit", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_transmit", "failure", e, esp_err_to_name(e));
  }
  return r;
}

/*
    esp_err_t i2c_master_transmit_receive(
        i2c_master_dev_handle_t i2c_dev,
        const uint8_t *write_buffer, size_t write_size,
        uint8_t *read_buffer, size_t read_size,
        int xfer_timeout_ms)
*/
bool Device::write_read(
      const uint8_t * write_buffer, size_t write_size,
      uint8_t * read_buffer, size_t read_size)
{
  //hex_dump(write_buffer, write_size);
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_transmit_receive(
      i2c_master_dev_handle_,
      write_buffer, write_size,
      read_buffer, read_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_transmit_receive", "failure", e, esp_err_to_name(e));
  } else {
    //hex_dump(read_buffer, read_size);
  }
  return r;
}


bool Adc::read(Register rn, uint16_t & v)
{
  const uint8_t write_data[1] = { uint8_t(rn) };
  uint8_t read_data[2] = { 0, 0 };
  const auto r = device_.write_read(
      write_data, sizeof(write_data),
      read_data, sizeof(read_data));
  if (r) {
    v = (uint16_t(read_data[0]) << 8) | read_data[1];
    //LOG("rn=%u v=0x%04X/%u/%i", unsigned(rn), v, v, int16_t(v));
  }
  return r;
}

bool Adc::read_repeat(uint16_t & v)
{
  uint8_t read_data[2] = { 0, 0 };
  const auto r = device_.read(read_data, sizeof(read_data));
  if (r) {
    v = (uint16_t(read_data[0]) << 8) | read_data[1];
    //LOG("v=0x%04X/%u/%i", v, v, int16_t(v));
  }
  return r;
}

bool Adc::write(Register rn, uint16_t v)
{
  const uint8_t write_data[3] = { uint8_t(rn), v >> 8, v & 0xFF };
  const auto r = device_.write(write_data, sizeof(write_data));
#if 0
  if (r) {
    LOG("rn=%u v=0x%04X/%u/%i", unsigned(rn), v, v, int16_t(v));
  }
#endif
  return r;
}


namespace adc_regs {

void dump(Config c)
{
#if 1
  {
    const auto v = *(uint16_t*) &c;
    LOG("c=0x%04X/%u/%i", v, v, int16_t(v));
  }
#endif

  static const char * const dr_names[] = {
    "8", "16", "32", "64", "128", "250", "475", "860"
  };

  LOG(
    "OS=%u"
    " MUX=%u"
    " PGA=%u"
    " MODE=%u"
    " DR=%u/'%s'"
    " COMP_MODE=%u"
    " COMP_POL=%u"
    " COMP_LAT=%u"
    " COMP_QUE=%u",
    c.OS,
    c.MUX,
    c.PGA,
    c.MODE,
    c.DR, dr_names[c.DR],
    c.COMP_MODE,
    c.COMP_POL,
    c.COMP_LAT,
    c.COMP_QUE
  );
}

} // namespace adc_regs


Adc adc = {
  .device_ = device,
};

bool Adc::init()
{
  if (init_done_) { return true; }

  if (!bus.init()) { return false; }
  if (!device.init()) {
    bus.down();
    return false;
  }
  init_done_ = true;
  return true;
}

void Adc::down()
{
  if (!init_done_) { return; }

  device.down();
  bus.down();

  init_done_ = false;
}

} // namespace i2c
} // namespace nc

namespace nc {
namespace i2c {

bool Adc::start_single_conversion(unsigned channel)
{
  const adc_regs::Config c = {
    .COMP_QUE = 3,
    .COMP_LAT = 0,
    .COMP_POL = 0,
    .COMP_MODE = 0,
    .DR = 7,
    .MODE = 1,
    //.PGA = 2,
    .PGA = 0,
    .MUX = 4 | channel,
    .OS = 1,
  };
  //LOG("sizeof(adc_regs::Config)=%zu", sizeof(adc_regs::Config));
  //adc_regs::dump(c);

  const auto v = *(uint16_t*) &c;
  const auto r = write(Register::Config, v);
  return r;
}

} // namespace i2c
} // namespace nc

#define I_Meter_Measurements_STRLEN ( \
  CONSTSTRLEN("/I2C/ADC/iMeter/DATA") \
  \
  + ( \
    + CONSTSTRLEN(" 0") \
    + CONSTSTRLEN(" 61440000") \
  ) * I_METERS_COUNT \
  \
  + CONSTSTRLEN(" XX") \
  + CONSTSTRLEN("\n") \
)

void i2c_adc_report(const char * args)
{
  (void) args;

  char buffer[I_Meter_Measurements_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/I2C/ADC/iMeter/DATA");
  b << iMeters;
  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

void i2c_adc_run(const char * args)
{
  (void) args;
  iMeters.run();
}

void i2c_adc_stats(const char * args)
{
  (void) args;
  ::nc::i2c::stats::report();
}

// 6.144 / 32768. = 0.0001875
// const auto volts = double(v) * 6.144 / 32768.;

::nc::mk::Buffer &
I_Meter::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << status
    << ' '
    << value * 1875;
}

I_Meters iMeters;

void I_Meters::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(init_failed)
    CASE_CALL(idle)
    CASE_CALL(transfer_init)
    CASE_CALL(transfer_wait)
    CASE_CALL(transfer_timeout)
    CASE_CALL(next_channel)
    CASE_CALL(run_finished)
#undef CASE_CALL
    default:
      //LOG("ERROR: Unexpected state=%u", state_);
      LOG("ERROR: Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

// Config.OS == 0 -- ADC performs conversion
bool I_Meters::poll(bool & ready)
{
  ::nc::i2c::adc_regs::Config c = {};
  const auto r = ::nc::i2c::adc.read_repeat( *(uint16_t*) &c);
  ++stats.poll[r];
  ready = r && c.OS;
  return r;
}

bool I_Meters::read(uint16_t & v)
{
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
  ++stats.read[r];
  return r;
}

bool I_Meters::start()
{
  const auto r = ::nc::i2c::adc.start_single_conversion(channel_);
  ++stats.start[r];
  return r;
}

void I_Meters::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    run_finished_ = false;

    start_time_ms_ = now_ms;
    channel_ = 0;

    state(State::transfer_init);
  }
}

void I_Meters::on_state_init(uint32_t now_ms)
{
  state( (::nc::i2c::adc.init()) ? State::idle : State::init_failed);
}

// Can be used to implement delay before init retry
void I_Meters::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
  state(State::idle);
}

void I_Meters::on_state_next_channel(uint32_t now_ms)
{
  ++channel_;
  if (channel_ < I_METERS_COUNT) {
    state(State::transfer_init);
  } else {
    state(State::run_finished);
  }
}

void I_Meters::on_state_run_finished(uint32_t now_ms)
{
  end_time_ms_ = now_ms;
  run_finished_ = true;
  state(State::idle);
}

const char * I_Meters::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "transfer_init",
    "transfer_wait",
    "transfer_timeout",
    "next_sensor",
    "run_finished",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void I_Meters::on_state_transfer_init(uint32_t now_ms)
{
  timeout_time_ms_ = now_ms + single_conversion_timeout_ms;

  state(
    (!start())
      ? State::next_channel
      : State::transfer_wait
  );
}

void I_Meters::on_state_transfer_timeout(uint32_t now_ms)
{
  LOG("WARNING: %s %s (timeout_time_ms=%u, now_ms=%u now_us=%" PRIu64 ")",
      "i2c ADC 'conversion'", "timeout", timeout_time_ms_, now_ms,
      esp_timer_get_time());

  state(State::next_channel);
}

/*
    After 'start_single_conversion()' 'index register' already refers to
    'Config' register and 'read_repeat()' may be used to read 'Config'
    register.
*/

void I_Meters::on_state_transfer_wait(uint32_t now_ms)
{
  bool ready;
  if (!poll(ready)) {
    state(State::next_channel);
    return;
  }

  if (!ready) {
    if (timeout_time_ms_ <= now_ms) {
      state(State::transfer_timeout);
    }
    return;
  }

  uint16_t v = 0;
  const auto r = read(v);
  if (r) {
    iMeters_[channel_].value = v;
    iMeters_[channel_].status.transfer_done = true;;
  }

  state(State::next_channel);
}

void I_Meters::state(State new_state)
{
#if defined iMeters_state_verbose
  const auto old_state_name = to_string(state_);
#endif
  state_ = new_state;
#if defined iMeters_state_verbose
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
#endif
}

::nc::mk::Buffer &
I_Meters::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << start
    << poll
    << read;
}

namespace nc {
namespace i2c {
namespace stats {

#define I_Meter_Stats_STRLEN ( \
  CONSTSTRLEN("/I2C/ADC/iMeter/STATS") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" 123465790") \
  + CONSTSTRLEN(" XX") \
  + CONSTSTRLEN("\n") \
)

void report()
{
  char buffer[I_Meter_Stats_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/I2C/ADC/iMeter/STATS");

  b << iMeters.stats;

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace stats
} // namespace i2c
} // namespace nc

::nc::mk::Buffer &
I_Meter::Status::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << uint8_t(!transfer_done);
}

::nc::mk::Buffer &
I_Meters::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  for (unsigned i = 0; i < I_METERS_COUNT; ++i) {
    b << iMeters_[i];
  }
  return b;
}

::nc::mk::Buffer &
::nc::lgt::AnalogSignals::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  ::nc::lgt::to_buffer(b, voltages);
  ::nc::lgt::to_buffer(b, temperatures);
  return b;
}

::nc::mk::Buffer &
::nc::lgt::SensorData::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    //<< ' '
    //<< device_id
    << ' '
    << unsigned(vcc) * 125
    //<< ' '
    << analogSignals;
}

#if 0
::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const ::nc::lgt::Temperature temperature)
{
  return b
    << temperature;
}
#endif

::nc::mk::Buffer &
::nc::lgt::to_buffer(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Temperatures & temperatures)
{
  for (unsigned t = 0; t < ::nc::lgt::temperature_pins_COUNT; ++t) {
    for (unsigned g = 0; g < ::nc::lgt::temperature_groups_COUNT; ++g) {
      b << ' '
        << temperatures[t][g];
    }
  }
  return b;
}

#if 0
::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const ::nc::lgt::Voltage voltage)
{
  return b
    << voltage;
}
#endif

::nc::mk::Buffer &
::nc::lgt::to_buffer(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Voltages & voltages)
{
  for (unsigned i = 0; i < ::nc::lgt::voltage_pins_COUNT; ++i) {
    b << ' '
      << voltages[i];
  }
  return b;
}

void stats_disable(const char * args)
{
  (void) args;
  ::nc::loop::stats_.disable();
}

void stats_enable(const char * args)
{
  (void) args;
  ::nc::loop::stats_.enable();
}

namespace nc {
namespace loop {

void Stats::on_loop(uint32_t now_ms)
{
  ++loop_count_;

  if (disabled_) { return; }

  if ((report_stats_time_ms + report_stats_period_ms) <= now_ms) {
    report_stats_time_ms = now_ms;
    report(now_ms);
  }

  if ((report_config_time_ms + report_config_period_ms) <= now_ms) {
    report_config_time_ms = now_ms;
    report_config();
  }
}

} // namespace loop
} // namespace nc

namespace nc {
namespace loop {

void Stats::report(uint32_t now_ms) const
{
  Serial.print(device_tag);
  Serial.print(" now_ms="); Serial.print(now_ms);
  Serial.print(" loop_count="); Serial.println(loop_count_);
}

} // namespace loop
} // namespace nc

namespace nc {
namespace loop {

Stats stats_;
  
} // namespace loop
} // namespace nc

using namespace nc;

void monitor_I(const char * args)
{
  monitor_.disable_.I_ = !monitor_.disable_.I_;
  LOG("disable_={raw_=%i I_=%i V_=%i T_=%i}",
      monitor_.disable_.raw_,
      monitor_.disable_.I_,
      monitor_.disable_.V_,
      monitor_.disable_.T_);
}

using namespace nc;

void monitor_period(const char * args)
{
  monitor_.pause_.period_ms_ = command_args_to_u32(args, nullptr);
  LOG("monitor_.pause_.period_ms_=%u", monitor_.pause_.period_ms_);
}

using namespace nc;

void monitor_raw(const char * args)
{
  monitor_.disable_.raw_ = !monitor_.disable_.raw_;
  LOG("disable_={raw_=%i I_=%i V_=%i T_=%i}",
      monitor_.disable_.raw_,
      monitor_.disable_.I_,
      monitor_.disable_.V_,
      monitor_.disable_.T_);
}

using namespace nc;

void monitor_start(const char * args)
{
  monitor_.start();
}

using namespace nc;

void monitor_stop(const char * args)
{
  monitor_.stop();
}

using namespace nc;

void monitor_T(const char * args)
{
  monitor_.disable_.T_ = !monitor_.disable_.T_;
  LOG("disable_={raw_=%i I_=%i V_=%i T_=%i}",
      monitor_.disable_.raw_,
      monitor_.disable_.I_,
      monitor_.disable_.V_,
      monitor_.disable_.T_);
}

using namespace nc;

void monitor_V(const char * args)
{
  monitor_.disable_.V_ = !monitor_.disable_.V_;
  LOG("disable_={raw_=%i I_=%i V_=%i T_=%i}",
      monitor_.disable_.raw_,
      monitor_.disable_.I_,
      monitor_.disable_.V_,
      monitor_.disable_.T_);
}

namespace nc {

Monitor monitor_;

} // namespace nc

namespace nc {

void Monitor::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(idle)
    CASE_CALL(start)
    CASE_CALL(stop)
    CASE_CALL(measure_init)
    CASE_CALL(measure_wait)
    CASE_CALL(measure_done)
    CASE_CALL(measure_report)
    CASE_CALL(measure_timeout)
    CASE_CALL(pause_init)
    CASE_CALL(pause_wait)
    CASE_CALL(pause_done)
#undef CASE_CALL
    default:
      LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

} // namespace nc

namespace nc {

static float V_to_I(float V)
{
  return V;
}

static float scale_I(uint16_t v)
{
  return V_to_I(float(v) * 0.0001875);
}

static float scale_V(int16_t v)
{
  return float(v) / 1000.;
}

static float V_to_T(float V)
{
  return (V - 0.4) / 0.019;
}

static float scale_T(uint16_t v)
{
  return V_to_T(float(v) / 1000.);
}

static void report_I_raw()
{
  Serial.print(" I0raw:");
  Serial.print(iMeters.iMeters_[0].value);

  Serial.print(" I1raw:");
  Serial.print(iMeters.iMeters_[1].value);
}

static void report_I()
{
  Serial.print(" I0:");
  Serial.print(scale_I(iMeters.iMeters_[0].value),7);

  Serial.print(" I1:");
  Serial.print(scale_I(iMeters.iMeters_[1].value),7);
}

static void report_V(
    unsigned spi_sensor_n,
    unsigned index)
{
  const auto & deviceData = ::nc::spi::sensors.sensors_[spi_sensor_n].raw.data;
  const auto & analogSignals = deviceData.analogSignals;
  const auto & voltages = analogSignals.voltages;
  Serial.print(" V");
  Serial.print(spi_sensor_n + 1);
  Serial.print(index);
  Serial.print(":");
  Serial.print(scale_V(voltages[index]),3);
}

static void report_T(
    unsigned spi_sensor_n,
    unsigned group,
    unsigned index)
{
  const auto & deviceData = ::nc::spi::sensors.sensors_[spi_sensor_n].raw.data;
  const auto & analogSignals = deviceData.analogSignals;
  const auto & temperatures = analogSignals.temperatures;
  Serial.print(" T");
  Serial.print(spi_sensor_n + 1);
  Serial.print(index);
  Serial.print(group);
  Serial.print(":");
  Serial.print(scale_T(temperatures[group][index]));
}

void Monitor::report_VT(unsigned spi_sensor_n) const
{
  if (!disable_.V_) {
    for (unsigned i = 0; i < ::nc::lgt::voltage_pins_COUNT; ++i) {
      report_V(spi_sensor_n, i);
    }
  }

  if (!disable_.T_) {
    for (unsigned i = 0; i < ::nc::lgt::temperature_pins_COUNT; ++i) {
      report_T(spi_sensor_n, 0, i);
      report_T(spi_sensor_n, 1, i);
    }
  }
}

void Monitor::report() const
{
  if (!disable_.I_) {
    if (!disable_.raw_) {
      report_I_raw();
    }
    report_I();
  }

  ;
  for (unsigned sensor_n = 0; sensor_n < SPI_SENSORS_COUNT; ++sensor_n) {
    if (!::nc::spi::sensors.offline(sensor_n)) {
      report_VT(sensor_n);
    }
  }

  Serial.println("");
}

} // namespace nc

namespace nc {

void Monitor::on_state_idle(uint32_t now_ms)
{
  (void) now_ms;

  if (start_) {
    state(State::start);
  }
}

} // namespace nc

namespace nc {

void Monitor::on_state_start(uint32_t now_ms)
{
  (void) now_ms;

  start_ = false;

  active_ = true;

  pause_.end_time_ms_ = now_ms;

  state(State::measure_init);
}

} // namespace nc

namespace nc {

void Monitor::on_state_stop(uint32_t now_ms)
{
  (void) now_ms;

  stop_ = false;
  active_ = false;
  state(State::idle);
}

} // namespace nc

namespace nc {

void Monitor::on_state_measure_init(uint32_t now_ms)
{
  measure_.timeout_time_ms_ = now_ms + measure_.timeout_ms;

  ::nc::spi::sensors.run();
  iMeters.run();

  state(State::measure_wait);
}

} // namespace nc

namespace nc {

void Monitor::on_state_measure_wait(uint32_t now_ms)
{
  if (stop_) {
    state(State::stop);
  } else
  if (::nc::spi::sensors.run_finished()
  && iMeters.run_finished()) {
    state(State::measure_done);
  } else
  if (measure_.timeout_time_ms_ <= now_ms) {
    LOG("ERROR: measurements timeout");
    state(State::measure_timeout);
  }
}

} // namespace nc

namespace nc {

void Monitor::on_state_measure_done(uint32_t now_ms)
{
  (void) now_ms;

  state(State::measure_report);
}

} // namespace nc

namespace nc {

void Monitor::on_state_measure_report(uint32_t now_ms)
{
  (void) now_ms;

  report();

  state(State::pause_init);
}

} // namespace nc

namespace nc {

void Monitor::on_state_measure_timeout(uint32_t now_ms)
{
  (void) now_ms;

  state(State::stop);
}

} // namespace nc

namespace nc {

void Monitor::on_state_pause_init(uint32_t now_ms)
{
  pause_.end_time_ms_ += pause_.period_ms_;

  state(State::pause_wait);
}

} // namespace nc

namespace nc {

void Monitor::on_state_pause_wait(uint32_t now_ms)
{
  if (stop_) {
    state(State::stop);
  } else
  if (pause_.end_time_ms_ <= now_ms) {
    state(State::pause_done);
  }
}

} // namespace nc

namespace nc {

void Monitor::on_state_pause_done(uint32_t now_ms)
{
  (void) now_ms;

  state(State::measure_init);
}

} // namespace nc

namespace nc {

const char * Monitor::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "start",
    "stop",
    "measure_init",
    "measure_wait",
    "measure_done",
    "measure_report",
    "measure_timeout",
    "pause_init",
    "pause_wait",
    "pause_done",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

} // namespace nc

namespace nc {

void Monitor::state(State new_state)
{
#if defined nc_Monitor_state_verbose
  const auto old_state_name = to_string(state_);
#endif
  state_ = new_state;
#if defined nc_Monitor_state_verbose
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
#endif
}

} // namespace nc

void pin_dump(const char * args)
{
  unsigned pin = command_args_to_u32(args, nullptr);
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
  unsigned pin = command_args_to_u32(args, nullptr);
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
  unsigned pin = command_args_to_u32(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    unsigned v = command_args_to_u32(tail, nullptr);
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
  unsigned pin = command_args_to_u32(args, &tail);
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
    unsigned v = command_args_to_u32(tail, nullptr);
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
  unsigned pin = command_args_to_u32(args, &tail);
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
    unsigned v = command_args_to_u32(tail, nullptr);
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
      } else {
        profileStep->dump();
      }
    }
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

void report_config()
{
  Serial.print("device: '"); Serial.print(device_tag); Serial.println("'");
  ::nc::spi::report_build_config();
}

void serial_write_buffer(
    const char * file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used)
{
  if (used < buffer_size) {
    const size_t availableForWrite_ = cnDevice.availableForWrite();
    if (availableForWrite_ < used) {
      log(file, line, function,
          "WARNING: availableForWrite_=%zu < used=%zu",
          availableForWrite_, used);
    }
    Serial.write(buffer, used);
#if 0
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

namespace nc {

shared_buffers_t shared_buffers;

} // namespace nc

const char * skip_whitespace(const char * p)
{
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  if (!p) { return p; }

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
  if (!p) { return p; }

  while (' ' == *p || '\t' == *p) {
    ++p;
    //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  }
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  return p;
}

namespace nc {
namespace cli {

void spi_config(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    const uint32_t v = command_args_to_u32(args, nullptr);
    ::nc::spi::sensors.offline_config(v);
  }
  ::nc::spi::sensors.offline_config_dump();
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_data(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.spi_data_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/DATA");

#if 1
  b << ::nc::spi::sensors;
#else
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    //b << ::nc::spi::sensors.sensors_[i].data;
    ::nc::spi::sensors.sensors_[i].to_buffer(b);
  }
#endif

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_data_raw(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.spi_raw_data_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/RAW/DATA");

  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    //b << ::nc::spi::sensors.sensors_[i].raw.data;
    b << ::nc::spi::sensors.sensors_[i].raw;
  }

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_dump(const char * args)
{
  (void) args;
  ::nc::spi::stats::report();
  spi_report_data();
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_run(const char * args)
{
  (void) args;
  ::nc::spi::sensors.run();
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void spi_stats(const char * args)
{
  (void) args;
  ::nc::spi::stats::report();
}

} // namespace cli
} // namespace nc

//#define SPI_TRANS_TIME_STATS


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

namespace nc {
namespace spi {
namespace hw {

void Chip::down()
{
  if (!active_) { return; }

  const auto r = (ESP_OK == spi_slave_free(SPI_DEVICE));
  if (!r) {
    LOG("ERROR: '%s()' failure", "spi_slave_free");
  }
  active_ = false;
}

} // namespace hw
} // namespace spi
} // namespace nc

namespace nc {
namespace spi {
namespace hw {

bool Chip::init()
{
  //LOG("");
  if (active_) { return true; }

  const auto r = (ESP_OK == spi_slave_initialize(SPI_DEVICE,
      &spi_bus_config, &spi_slave_interface_config, SPI_DMA_DISABLED));
  active_ = r;
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_initialize");
  }
  return r;
}

} // namespace hw
} // namespace spi
} // namespace nc

const uint8_t spi_pins[] = {
  SPI_GPIO_MOSI,
  SPI_GPIO_SCLK,
  SPI_GPIO_CS,
};

// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
void spi_post_setup_cb(spi_slave_transaction_t * transaction)
{
  auto sensors = static_cast<::nc::spi::Sensors*>(transaction->user);
  ++sensors->stats.post_setup_cb_count;

  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_ACTIvE);
}

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of transfer.
void spi_post_trans_cb(spi_slave_transaction_t * transaction)
{
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  auto sensors = static_cast<::nc::spi::Sensors*>(transaction->user);
  ++sensors->stats.post_trans_cb_count;
  sensors->hw.active_ = false;
}

/*
    Currently only one transaction is used.

    Multiple transactions submitted to driver can improve performance but will
    complicate error handling (for exampe in case of timeout).
*/
//#define SPI_QUEUE_SIZE SPI_SENSORS_COUNT
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

void spi_mr_mux(unsigned spi_sensor_n)
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, (spi_sensor_n >> i) & 1);
  }
}

const uint8_t spi_mr_mux_pins[SPI_MR_MUX_GPIO_COUNT] = {
  SPI_MR_MUX_0_GPIO,
  SPI_MR_MUX_1_GPIO,
  SPI_MR_MUX_2_GPIO,
  SPI_MR_MUX_3_GPIO,
};

void spi_mr_mux_pins_setup()
{
  LOG("");
  for (unsigned i = 0; i < ARRAY_SIZE(spi_mr_mux_pins); ++i) {
    const auto spi_mr_mux_pin = gpio_num_t(spi_mr_mux_pins[i]);
    gpio_set_level(spi_mr_mux_pin, 0);
    gpio_set_direction(spi_mr_mux_pin, GPIO_MODE_INPUT_OUTPUT);
  }
}

void spi_mr_pin_setup()
{
  LOG("");
  const auto spi_mr_pin = gpio_num_t(SPI_MR_GPIO);
  gpio_set_level(spi_mr_pin, 1);
  gpio_set_direction(spi_mr_pin, GPIO_MODE_INPUT_OUTPUT);
}

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

namespace nc {
namespace spi {

void report_build_config()
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
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

uint8_t Sensor::Raw::crc() const
{
  return calcCRC8(
    (const uint8_t *) &data,
    LGT8F_SPI_DATA_SIZE - sizeof(data.crc));
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

bool Sensor::Raw::is_crc_valid_impl() const
{
  return data.crc == crc();
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensor::Raw::reset()
{
  memset(tx_buffer, 0, sizeof(tx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  status.transfer_done = false;
  status.rx_bits = 0;
  status.crc_valid_ = false;
}

} // namespace spi
} // namespace nc

::nc::mk::Buffer &
::nc::spi::Sensor::Raw::Status::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  union {
    struct {
      uint8_t transfer:1;
      uint8_t bits:1;
      uint8_t crc:1;
    } as_bits;
    uint8_t as_byte;
  } errors = {
    .as_bits = {
      .transfer = !transfer_done,
      .bits = !rx_bits_valid(),
      .crc = !is_crc_valid(),
    },
  };
  return b
    << ' '
    << errors.as_byte;
}

::nc::mk::Buffer &
::nc::spi::Sensor::Raw::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << status
    << data;
}

namespace nc {
namespace spi {

void Sensor::Raw::update_crc_valid()
{
  status.crc_valid_ = is_crc_valid_impl();
}

} // namespace spi
} // namespace nc

//namespace nc {
//namespace spi {

::nc::mk::Buffer &
::nc::spi::Sensor::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
/*
  LOG("&transfer=%p counter: [0]=%" PRIu64 " [1]=%" PRIu64 "",
      &transfer,
      transfer.counter_[0],
      transfer.counter_[1]);
  hex_dump( (const uint8_t *) &transfer, sizeof(transfer));
*/

  return b
    << transfer
    << rx_bits
    << crc;
}

//} // namespace spi
//} // namespace nc

::nc::mk::Buffer &
::nc::spi::Sensor::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << raw.status
    << data;
}

namespace nc {
namespace spi {

void Sensor::update_data()
{
  if (raw.status.transfer_done
  && raw.status.rx_bits_valid()
  && raw.status.is_crc_valid()) {
    data = raw.data;
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

/*
    Warning: 'spi_slave_queue_trans()' does not make copy of
    'spi_slave_transaction_t'. Instead it keeps it's address and uses it to
    store number of actually transferred bits into 'trans_len' field. It means
    that memory occupied by 'spi_slave_transaction_t' must be preserved until
    the end of transmission which is notified by 'post_trans_cb' callback.
*/
//static
//spi_slave_transaction_t Sensors::hw::transaction_;

bool Sensors::hw_transfer_start()
{
  //sensors_[sensor_n_].raw.reset();
  spi_mr_mux(sensor_n_);

  if (!hw.chip_.init()) { return false;}

  hw.transaction_ = spi_slave_transaction_t {
    //.flags = 0,
    .length = SPI_DATA_SIZE * 8, // number of bits not bytes
    // trans_len -- used by driver to return actual number of received bits
    //.trans_len = 0,
    .tx_buffer = sensors_[sensor_n_].raw.tx_buffer,
    .rx_buffer = sensors_[sensor_n_].raw.rx_buffer,
    // void * user // User-defined variable.
    .user = this,
  };
  const auto r =
      (ESP_OK == spi_slave_queue_trans(SPI_DEVICE, &hw.transaction_, 0));
  hw.active_ = r;
  if (!r) {
    LOG("ERROR: %s() failure", "spi_slave_queue_trans");
    hw.chip_.down();
  }
  return r;
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::offline_config(uint32_t v)
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    offline_[i] = true;
  }
  //LOG("v=0x%X/%u", v, v);
  while (v) {
    unsigned i = v % 10;
    //LOG("v=0x%X/%u", i, i);
    if (i) {
      --i;
      if (i < SPI_SENSORS_COUNT) {
        offline_[i] = false;
      }
    }
    v /= 10;
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::offline_config_dump() const
{
  char s[SPI_SENSORS_COUNT+1];
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    s[i] = (offline_[i]) ? '.' : 'X';
  }
  s[SPI_SENSORS_COUNT] = '\0';
  LOG("s='%s'", s);
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(init_failed)
    CASE_CALL(idle)
    CASE_CALL(transfer_init)
    CASE_CALL(transfer_wait)
    CASE_CALL(transfer_timeout)
    CASE_CALL(next_sensor)
    CASE_CALL(run_finished)
#undef CASE_CALL
    default:
      LOG("ERROR: Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::skip_offline()
{
  //LOG("sensor_n_=%u", sensor_n_);
  while (sensor_n_ < SPI_SENSORS_COUNT
      && offline_[sensor_n_]) {
    ++sensor_n_;
    //LOG("sensor_n_=%u", sensor_n_);
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_idle(uint32_t now_ms)
{
  if (run_) {
    run_ = false;
    run_finished_ = false;

    transfer_start_time_ms_ = now_ms;
    sensor_n_ = 0;

    state(State::transfer_init);
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_init(uint32_t now_ms)
{
  (void) now_ms;
  state( (hw.chip_.init()) ? State::idle : State::init_failed);
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

// Can be used to implement delay before init retry
void Sensors::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
  state(State::idle);
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_next_sensor(uint32_t now_ms)
{
  ++sensor_n_;
  if (sensor_n_ < SPI_SENSORS_COUNT) {
    state(State::transfer_init);
  } else {
    state(State::run_finished);
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_run_finished(uint32_t now_ms)
{
  transfer_end_time_ms_ = now_ms;
  run_finished_ = true;
  state(State::idle);
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

const char * Sensors::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "transfer_init",
    "transfer_wait",
    "transfer_timeout",
    "next_sensor",
    "run_finished",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_transfer_init(uint32_t now_ms)
{
  sensors_[sensor_n_].raw.reset();

  if (offline_[sensor_n_]) {
    state(State::next_sensor);
    return;
  }

  timeout_time_ms_ = now_ms + single_transfer_timeout_ms;
  if (!hw_transfer_start()) {
    ++sensors_[sensor_n_].stats.transfer.ng();
    state(State::next_sensor);
  } else {
    state(State::transfer_wait);
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_transfer_timeout(uint32_t now_ms)
{
  LOG("WARNING: spi sensor %u transfer timeout", sensor_n_);

  // make sure MR pin is inactive
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  hw.chip_.down();

  state(State::next_sensor);
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::on_state_transfer_wait(uint32_t now_ms)
{
  if (!hw.active_) {
    auto & sensor = sensors_[sensor_n_];

    sensor.raw.status.transfer_done = true;
    sensor.raw.status.rx_bits = hw.rx_bits();
    sensor.raw.update_crc_valid();

    sensor.update_data();

    ++sensor.stats.transfer.ok();
    ++sensor.stats.rx_bits[sensor.raw.status.rx_bits_valid()];
    ++sensor.stats.crc[sensor.raw.status.is_crc_valid()];

    state(State::next_sensor);
  } else
  if (timeout_time_ms_ <= now_ms) {
    ++sensors_[sensor_n_].stats.transfer.ng();
    state(State::transfer_timeout);
  }
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

void Sensors::state(State new_state)
{
#if defined spi_Sensors_state_verbose
  const auto old_state_name = to_string(state_);
#endif
  state_ = new_state;
#if defined spi_Sensors_state_verbose
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
#endif
}

} // namespace spi
} // namespace nc

namespace nc {
namespace spi {

::nc::mk::Buffer &
Sensors::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << post_setup_cb_count
    << ' '
    << post_trans_cb_count;
}

::nc::mk::Buffer &
Sensors::stats_to_buffer(::nc::mk::Buffer & b) const noexcept
{
  stats.to_buffer(b);
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    b << sensors_[i].stats;
  }
  return b;
}

} // namespace spi
} // namespace nc

::nc::mk::Buffer &
::nc::spi::Sensors::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    b << ::nc::spi::sensors.sensors_[i];
  }
  return b;
}

namespace nc {
namespace spi {

Sensors sensors;

} // namespace spi
} // namespace nc

void spi_setup()
{
  LOG("");
  spi_mr_mux_pins_setup();
  spi_mr_pin_setup();
  LOG("");
}

void analog_report_one(const ::nc::lgt::SensorData & sensorData, unsigned i)
{
  const auto analogSignal = sensorData.analogSignals.all[i];
  Serial.print(" ");
  Serial.print(analogSignal * 0.001, 3);
}

void analog_report_all(const ::nc::lgt::SensorData & sensorData)
{
  for (unsigned i = 0; i < ::nc::lgt::ALL_ANALOG_SIGNALS_COUNT; ++i) {
    analog_report_one(sensorData, i);
  }
  Serial.println("");
}

void SensorDevice_Data_report_one(unsigned i)
{
  const auto & sensor = ::nc::spi::sensors.sensors_[i];
  const auto & sensorData = sensor.raw.data;

#if 1
  const auto crc = sensor.raw.crc();
  LOG(". crc=0x%02X %s", crc, (sensorData.crc == crc) ? "OK" : "NG");
#endif
  hex_dump( (const uint8_t *)&sensorData, sizeof(sensorData));

  Serial.print(". . ");
  Serial.print(sensorData.vcc);
  Serial.print("/");
  Serial.print(sensorData.vcc * 0.00125, 4);
  Serial.print(" |");
  analog_report_all(sensorData);
}

void spi_report_data()
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    SensorDevice_Data_report_one(i);
  }
}

namespace nc {
namespace spi {
namespace stats {

void report()
{
  auto & buffer = ::nc::shared_buffers.spi_stats_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/SPI/SENSORS/STATS");

  ::nc::spi::sensors.stats_to_buffer(b);

  b.printf(" XX");
  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

} // namespace stats
} // namespace spi
} // namespace nc

namespace nc {
namespace stats {

::nc::mk::Buffer &
OkNg::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << ok()
    << ' '
    << ng();
}

} // namespace stats
} // namespace nc

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

void test_abort(const char * args)
{
  (void) args;
  test.abort();
}

void test_dump(const char * args)
{
  test.dump();
}

void test_run(const char * args)
{
  (void) args;
  test.run();
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
    CASE_CALL(measure_init)
    CASE_CALL(measure_wait)
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

#define TEST_STEP_DATA_STRLEN ( \
  CONSTSTRLEN("/TEST/DATA") \
  \
  + CONSTSTRLEN(" 99") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" 99999") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + CONSTSTRLEN(" 1234567890") \
  + CONSTSTRLEN(" 1234567890") \
  \
  + ( \
    + CONSTSTRLEN(" 61440000") \
  ) * I_METERS_COUNT \
  \
  + ( \
    CONSTSTRLEN("  123456") \
    + CONSTSTRLEN(" 12345") * ::nc::lgt::ALL_ANALOG_SIGNALS_COUNT \
  ) * SPI_SENSORS_COUNT \
  \
  + CONSTSTRLEN("  XX") \
  + CONSTSTRLEN("\n") \
)

void Test::report_step_data(
    uint32_t now_ms,
    uint32_t test_start_time_ms) const
{
  char buffer[TEST_STEP_DATA_STRLEN+1];
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

//test_start_time_ms = 0; // FIXME: remove this line

  b.printf("/TEST/DATA");

  // SP SA EP
//b.printf(" step:");
  b.printf(" %2u", step_.count_);
  b.printf(" %10u", step_.start_time_planned_ms_ - test_start_time_ms);
  b.printf(" %10u", step_.start_time_actual_ms_ - test_start_time_ms);
  b.printf(" %10u", step_.end_time_planned_ms_ - test_start_time_ms);

  // SP SA EP
//b.printf(" sub_step:");
  b.printf(" %5u", sub_.step_.count_);
  b.printf(" %10u", sub_.step_.start_time_planned_ms_ - test_start_time_ms);
  b.printf(" %10u", sub_.step_.start_time_actual_ms_ - test_start_time_ms);
  b.printf(" %10u", sub_.step_.end_time_planned_ms_ - test_start_time_ms);

  // SA (aks sub step EA)
//b.printf(" measure:");
  b.printf(" %10u", measure_.start_time_actual_ms_ - test_start_time_ms);
  // 'now' (aka measure end)
  b.printf(" %10u", now_ms - test_start_time_ms);

//b.printf("  |");
  b << iMeters;
//b.printf("  |");
  b << ::nc::spi::sensors;

  b.printf("  XX\n");

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
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
      test_start_time_ms_               = now_ms;
      step_.start_time_planned_ms_      = now_ms;
      step_.end_time_planned_ms_        = now_ms; // FIXME: This line is for safety.
      sub_.step_.start_time_planned_ms_ = now_ms;
      sub_.step_.end_time_planned_ms_   = now_ms; // FIXME: This line is for safety.
#if 0
      LOG(". TEST test_start_time_ms_=%u", test_start_time_ms_);
      LOG(". TEST step_.start_time_planned_ms_=%u", step_.start_time_planned_ms_);
      LOG(". TEST step_.end_time_planned_ms_=%u", step_.end_time_planned_ms_);
      LOG(". TEST sub_.step_.start_time_planned_ms_=%u", sub_.step_.start_time_planned_ms_);
      LOG(". TEST sub_.step_.end_time_planned_ms_=%u", sub_.step_.end_time_planned_ms_);
#endif

      // --- 'step_0' ---
      step_.count_ = 0;

      state(State::step_init);
    }
  }
}

#define TEST_STEP_START_STRLEN CONSTSTRLEN("/TEST/STEP 99\n")

void Test::on_state_step_init(uint32_t now_ms)
{
  // --- 'step_init' ---
  step_.start_time_actual_ms_ = now_ms;

  char buffer[TEST_STEP_START_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  size_t used = safe_snprintf(buffer, buffer_size, "/TEST/STEP %2u\n", step_.count_);
  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);

  const ProfileStep * profileStep = &profileSteps_[step_.count_];

  // To reduce 'time drift' 'step planned end time' shall be relative to 'step
  // planned start time' instead of 'step actual start time'.
  // FIXME: Don't forget to update 'step_.start_time_planned_ms_' for 'next step'
  step_.end_time_planned_ms_ = step_.start_time_planned_ms_ + profileStep->step_ms;

  // set PS/EL configs
  change_PS_EL_configs(profileStep);

  // --- 'sub_step_0' ---
  sub_.step_.count_ = 0;
  sub_.step_.start_time_planned_ms_ = step_.start_time_planned_ms_;

  state(State::sub_step_init);
}

/*
    if !step_ms skip waiting and jump directly to 'measure'.

    '!step_ms' can be useful to do measurements at the beginning of the
    test immediately after changing PS and EL configs.

    if !sub_step_ms then next '*_sub_step_end' will be quickly executed
    just after few 'on_loop'.

    '!sub_step_ms' combined with non-zero 'step_ms' can be useful to do
    multiple repeated measurements at the beginning of the test immediately
    after changing PS and EL configs.
*/

void Test::on_state_sub_step_init(uint32_t now_ms)
{
  // --- 'sub_step_init' ---
  sub_.step_.start_time_actual_ms_ = now_ms;

  const ProfileStep * profileStep = &profileSteps_[step_.count_];

  // To reduce 'time drift' 'sub step planned end time' shall be relative to
  // 'sub step planned start time' instead of 'sub step actual start time'.
  // FIXME: Don't forget to update 'sub_.step_.start_time_planned_ms for 'next sub step'
  sub_.step_.end_time_planned_ms_ = sub_.step_.start_time_planned_ms_ + profileStep->sub_step_ms;

  state(State::sub_step_wait);
}

// Long running state. Check for user's abort request.

// 'sub_step_wait'
void Test::on_state_sub_step_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (sub_.step_.end_time_planned_ms_ <= now_ms) {
#if 0
    // --- 'sub_step_end' ---
    LOG("--- sub_.step_.count_=%u finished -- do measurements...", sub_.step_.count_);
#endif

    state(State::measure_init);
  }
}

// 'measure_init'
void Test::on_state_measure_init(uint32_t now_ms)
{
  // --- 'measure_init' ---
  measure_.start_time_actual_ms_ = now_ms;

  measure_.timeout_time_ms_ = now_ms + measure_.timeout_ms;

  ::nc::spi::sensors.run();
  iMeters.run();

  state(State::measure_wait);
}

// Long running state. Check for user's abort request.

// 'measure_wait'
void Test::on_state_measure_wait(uint32_t now_ms)
{
  if (abort_) {
    state(State::abort);
  } else
  if (::nc::spi::sensors.run_finished()
  && iMeters.run_finished()) {
    // --- 'measure end' ---
    measure_.end_time_actual_ms_ = now_ms;

    // report measurements
    report_step_data(now_ms, test_start_time_ms_);

    // start next sub step
    state(State::sub_step_next);
  } else
  if (measure_.timeout_time_ms_ <= now_ms) {
    LOG("ERROR: measurements timeout");
    state(State::abort);
  }
}

void Test::on_state_sub_step_next(uint32_t now_ms)
{
  // --- 'sub_step_next' ---

  //const ProfileStep * profileStep = &profileSteps_[step_.count_];
  ++sub_.step_.count_;
  //LOG(". sub_.step_.count_=%u", sub_.step_.count_);
#if 1
  if (sub_.step_.count_ > sub_step_count_max_) {
    LOG("ERROR: sub_.step_.count_=%u limit=%u exceeded",
        sub_.step_.count_, sub_step_count_max_);
    state(State::abort);
  }
#endif
  if (now_ms < step_.end_time_planned_ms_) {
    //LOG(". now_ms=%u < step_.end_time_planned_ms_=%u", now_ms, step_.end_time_planned_ms_);
    sub_.step_.start_time_planned_ms_ = sub_.step_.end_time_planned_ms_;
    state(State::sub_step_init);
  } else {
    //LOG("--- step_=%u finished -- start next step (change PS and EL configs) ...", step_.count_);
    state(State::step_next);
  }
}

void Test::on_state_step_next(uint32_t now_ms)
{
  // --- 'step_next' ---
  ++step_.count_;
  //LOG(". step_.count_=%u", step_.count_);
  if (step_.count_ < profile.steps_count_) {
    step_.start_time_planned_ms_ = step_.end_time_planned_ms_;
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

  uart_stats_report();
  ::nc::i2c::stats::report();
  ::nc::spi::stats::report();

  state(State::idle);
}

const char * Test::to_string(State state)
{
  static const char * const state_names[] = {
    "idle",
    "step_init",
    "sub_step_init",
    "sub_step_wait",
    "measure_init",
    "measure_wait",
    "sub_step_next",
    "step_next",
    "abort",
    "cleanup",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void Test::state(State new_state)
{
  const auto old_state_name = to_string(state_);
  state_ = new_state;
  const auto new_state_name = to_string(state_);
  //LOG("'%s' -> '%s'", old_state_name, new_state_name);
}

Test test;

void Test::dump() const
{
  hex_dump( (const uint8_t*)this, sizeof(*this));
  LOG("step.count_=%u"
      " sub_.step.count_=%u"
      " test_start_time_ms=%u"
      " sub_.step_.end_time_planned_ms=%u"
      " measure_.timeout_time_ms=%u"
      " run=%i"
      " abort=%i"
      " state=%u/%s",
      step_.count_,
      sub_.step_.count_,
      test_start_time_ms_,
      sub_.step_.end_time_planned_ms_,
      measure_.timeout_time_ms_,
      run_,
      abort_,
      state_, to_string(state_));
}

namespace nc {
namespace cli {

void uart_stats(const char * args)
{
  (void) args;
  uart_stats_report();
}

} // namespace cli
} // namespace nc

namespace nc {
namespace cli {

void uart_test(const char * args)
{
  const char * tail = args;
  //LOG("tail=%p/'%s'", tail, tail ? tail : "");
  //cnDevice.uartConfig_->finish_output();
  unsigned count = command_args_to_u32(tail, &tail);
  unsigned width = command_args_to_u32(tail, nullptr);
  LOG("count=%u width=%u", count, width);
  //cnDevice.uartConfig_->finish_output();

  uint64_t time_12_us_min = uint64_t(-1);
  uint64_t time_23_us_min = uint64_t(-1);
  uint64_t time_13_us_min = uint64_t(-1);

  uint64_t time_12_us_max = 0;
  uint64_t time_23_us_max = 0;
  uint64_t time_13_us_max = 0;

  uint64_t time_0_us = esp_timer_get_time();
  static constexpr uint64_t el_period_us = 600000;
  uint64_t el_time_us = time_0_us + el_period_us;

  for (unsigned step = 0; step < count; ++step) {
    uint64_t time_1_us = esp_timer_get_time();

    auto & buffer = ::nc::shared_buffers.uart_test_buffer;
    size_t buffer_size = sizeof(buffer);
    ::nc::mk::Buffer b(buffer, buffer_size);

    const auto time_us = time_1_us - time_0_us;
    b.printf("/UART/TEST %5u %6" PRIu64 ".%06u ", step, time_us / 1000000, unsigned(time_us % 1000000));

    static const char chars[] =
        ".0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (unsigned i = 0; i < width; ++i) {
#if 1
      b.put_char(chars[i % (sizeof(chars) - 1)]);
#else
      b.printf("%c", chars[i % (sizeof(chars) - 1)]);
#endif
    }

    b.printf("|");
    b.printf("\n");
    uint64_t time_2_us = esp_timer_get_time();

    size_t tx_available_1 = cnDevice.availableForWrite();
    SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
    size_t tx_available_2 = cnDevice.availableForWrite();
    //cnDevice.uartConfig_->finish_output();

    uint64_t time_3_us = esp_timer_get_time();

    LOG("tx_available_1=%zu tx_available_2=%zu", tx_available_1, tx_available_2);

    uint64_t time_12_us = time_2_us - time_1_us;
    uint64_t time_23_us = time_3_us - time_2_us;
    uint64_t time_13_us = time_3_us - time_1_us;
    LOG("time %" PRIu64 " %" PRIu64 " %" PRIu64, time_12_us, time_23_us, time_13_us);
    //cnDevice.uartConfig_->finish_output();

    if (time_12_us_min > time_12_us) { time_12_us_min = time_12_us; }
    if (time_23_us_min > time_23_us) { time_23_us_min = time_23_us; }
    if (time_13_us_min > time_13_us) { time_13_us_min = time_13_us; }

    if (time_12_us_max < time_12_us) { time_12_us_max = time_12_us; }
    if (time_23_us_max < time_23_us) { time_23_us_max = time_23_us; }
    if (time_13_us_max < time_13_us) { time_13_us_max = time_13_us; }

    if (el_time_us < time_3_us) {
      el_id(nullptr);
      el_time_us = time_3_us + el_period_us;
    }
    elDevice.on_loop(time_3_us / 1000);
  }
  LOG("time min %" PRIi64 " %" PRIi64 " %" PRIi64, time_12_us_min, time_23_us_min, time_13_us_min);
  LOG("time max %" PRIi64 " %" PRIi64 " %" PRIi64, time_12_us_max, time_23_us_max, time_13_us_max);

  uart_stats_report();
}

} // namespace cli
} // namespace nc

void uart_stats_report()
{
  auto & buffer = ::nc::shared_buffers.uart_stats_buffer;
  size_t buffer_size = sizeof(buffer);

  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/STATS/UART rx{available.max/FIFO/BUFFER/PARITY/FRAME} tx{available.{min/max}}");

  static const struct {
    const char * const tag;
    const UartDevice * const uartDevice;
  } uartDevices[] = {
    { "cn", &cnDevice, },
    { "ps", &psDevice, },
    { "el", &elDevice, },
  };;
  for (unsigned i = 0; i < ARRAY_SIZE(uartDevices); ++i) {
    b.printf(" %s %zi/%u/%u/%u/%u %zi/%zi", uartDevices[i].tag,
      uartDevices[i].uartDevice->stats_.availableForRead_max_,
      uartDevices[i].uartDevice->stats_.UART_FIFO_OVF_ERROR_,
      uartDevices[i].uartDevice->stats_.UART_BUFFER_FULL_,
      uartDevices[i].uartDevice->stats_.UART_PARITY_ERROR_,
      uartDevices[i].uartDevice->stats_.UART_FRAME_ERROR_,
      uartDevices[i].uartDevice->stats_.availableForWrite_min_,
      uartDevices[i].uartDevice->stats_.availableForWrite_max_
    );
  }

  b.printf("\n");

  SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
}

const UartConfig cn_uartConfig = {
  .tag =  "CN-u" MAKE_STRING(CN_UART_NUM),
  .hs = &Serial,
  .baud = CN_UART_BAUD,

//.rxBufferSize = 0,
//.rxBufferSize = 0x200,
//.rxBufferSize = 0x400,
  .rxBufferSize = 0x4000,

//.txBufferSize = 0x800,
//.txBufferSize = 0x1000,
  .txBufferSize = 0x4000,

  .n = 0,
  .rxd_gpio = -1, // '-1' == 'do not change'
  .txd_gpio = -1, // '-1' == 'do not change'

//.rx_fifo_irq_bytes = 60,
  .rx_fifo_irq_bytes = 110, // ??
//.rx_fifo_irq_bytes = 100, // OK
};

static const UartConfig ps_uartConfig = {
  .tag =  "PS-u" MAKE_STRING(PS_UART_NUM),
  .hs = &Serial1,
  .baud = PS_UART_BAUD,
  .rxBufferSize = 0,
  .txBufferSize = 0,
  .n = 1,
  .rxd_gpio = PS_UART_RX_GPIO,
  .txd_gpio = PS_UART_TX_GPIO,
};

static const UartConfig el_uartConfig = {
  .tag = "EL-u" MAKE_STRING(EL_UART_NUM),
  .hs = &Serial2,
  .baud = EL_UART_BAUD,
  .rxBufferSize = 0,
  .txBufferSize = 0,
  .n = 2,
  .rxd_gpio = EL_UART_RX_GPIO,
  .txd_gpio = EL_UART_TX_GPIO,
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

void PS_Device::clear_alerts() const
{
  const char * cmd = "TRIPRST";
  write_cmd(cmd);
}

CN_Device cnDevice = {
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

void ps_clear_alerts(const char * args)
{
  (void) args;
  psDevice.clear_alerts();
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

size_t UartDevice::availableForRead() const
{
  const auto available = uartConfig_->availableForRead();
  if (stats_.availableForRead_max_ < available) {
    stats_.availableForRead_max_ = available;
  }
  return available;
}

size_t UartDevice::availableForWrite() const
{
  const auto available = uartConfig_->availableForWrite();
  if (stats_.availableForWrite_max_ < available) {
    stats_.availableForWrite_max_ = available;
  }
  if (stats_.availableForWrite_min_ > available) {
    stats_.availableForWrite_min_ = available;
  }
  return available;
}

void UartDevice::on_loop(uint32_t now_ms)
{
  if (on_loop_readLine()) {
    udLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
    on_loop_readLine_reset();
  }
}

// returns '(eol_ || overflow_)'
bool UartDevice::on_loop_readLine()
{
  if (!eol_ && !overflow_) {
    auto available = availableForRead();
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

// assuming 'sizeof(buffer_) > 0'
void UartDevice::on_loop_readLine_reset()
{
  buffer_used_ = 0;
  overflow_ = false;
  eol_ = false;
  buffer_[0] = '\0'; // trailing '\0' for convenience
}

// TODO: Implement 'runnables_[]' switching

static Runnable * const runnables_[] = {
  &::nc::loop::stats_,
  &cnDevice,
  &psDevice,
  &elDevice,
  &iMeters,
  &::nc::spi::sensors,
  &test,
  &::nc::monitor_,
};

void loop()
{
  uint32_t now_ms = millis();

  for (unsigned i = 0; i < ARRAY_SIZE(runnables_); ++i) {
    runnables_[i]->on_loop(now_ms);
  }
}

void setup()
{
  xx_uartConfigs_init();
  //scope_sync_setup();
  spi_setup();
  report_config();

  cnDevice.attach_onReceiveError();
  psDevice.attach_onReceiveError();
  elDevice.attach_onReceiveError();
}
