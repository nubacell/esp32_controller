
struct IdMiniTask
  : CmdMiniTask
{
  IdMiniTask(
      MiniTaskObserver * miniTaskObserver,
      uint32_t timeout_time_ms,
      UartDevice * uartDevice,
      byte * buffer, size_t buffer_size)
    : CmdMiniTask(
        miniTaskObserver,
        timeout_time_ms,
        uartDevice,
        CMD_IDENTIFY,
        buffer, buffer_size)
  {
  }
};
