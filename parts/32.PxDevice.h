
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
