
struct CmdMiniTask
  : ReadLineMiniTask
{
  CmdMiniTask(
      MiniTaskObserver * miniTaskObserver,
      uint32_t timeout_time_ms,
      UartDevice * uartDevice,
      const char * cmd,
      byte * buffer, size_t buffer_size)
    : ReadLineMiniTask(miniTaskObserver, timeout_time_ms, uartDevice, buffer, buffer_size)
  {
    write_cmd(cmd);
  }
};
