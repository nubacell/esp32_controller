
struct SyncMiniTask
  : TimeoutMiniTask
  , UartMiniTask
{
  SyncMiniTask(
      MiniTaskObserver * miniTaskObserver,
      uint32_t timeout_time_ms,
      UartDevice * uartDevice)
    : TimeoutMiniTask(miniTaskObserver, timeout_time_ms)
    , UartMiniTask(miniTaskObserver, uartDevice)
  {}

  void discard_input()
  {
    while (availableForRead()) {
      read_byte(0);
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    discard_input();
    TimeoutMiniTask::on_loop(now_ms);
  }
};
