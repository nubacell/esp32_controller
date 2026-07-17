
#define PxLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct PxDevice
  : Runnable
{
  const UartConfig * uartConfig_;

  const char * const expected_id;
  uint32_t timeout_time_ms = 0;

  static constexpr size_t buffer_size = 64;
  byte buffer[buffer_size];
  LineReader lineReader_;

  void lineReader_on_eol(cb_arg_t cb_arg)
  {
    PxLOG("buffer='%s'", buffer);
  }
  static void lineReader_on_eol_proxy(cb_ctx_t cb_ctx, cb_arg_t cb_arg)
  {
    PxDevice * pxDevice = (PxDevice*) cb_ctx;
    pxDevice->lineReader_on_eol(cb_arg);
  }

  void lineReader_on_overflow(cb_arg_t cb_arg)
  {
    PxLOG("buffer='%s'", buffer);
  }
  static void lineReader_on_overflow_proxy(cb_ctx_t cb_ctx, cb_arg_t cb_arg)
  {
    PxDevice * pxDevice = (PxDevice*) cb_ctx;
    pxDevice->lineReader_on_overflow(cb_arg);
  }

  PxDevice(
      const UartConfig * uartConfig,
      const char * const expected_id_)
    : uartConfig_(uartConfig)
    , expected_id(expected_id_)
    , lineReader_(uartConfig,
        Callback(lineReader_on_eol_proxy, this),
        Callback(lineReader_on_overflow_proxy, this),
        buffer, sizeof(buffer))
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
      if (!uartConfig_->readLine_z(buffer, sizeof(buffer), timeout_ms)) {
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
    lineReader_.reset();

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
    lineReader_.on_loop(now_ms);
    if (timeout_time_ms <= now_ms) {
      PxLOG("%s %s", "sync/discard_input", "timeout/success");
      state = State(unsigned(state)+1); // ++state;
    }
  }

  void on_state_identify(uint32_t now_ms)
  {
    PxLOG("");
    lineReader_.reset();

    write_cmd(CMD_IDENTIFY);

    constexpr uint32_t identify_timeout_ms = 120;
    timeout_time_ms = now_ms + identify_timeout_ms;

    state = State(unsigned(state)+1); // ++state;
  }

  // assuming 'sizeof(buffer) > 0'
  void on_state_identify_read_answer(uint32_t now_ms)
  {
    //PxLOG("");
    lineReader_.on_loop(now_ms);
    if (lineReader_.eol() || lineReader_.overflow()) {
      state = State(unsigned(state)+1); // ++state;
    } else if (timeout_time_ms <= now_ms) {
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
