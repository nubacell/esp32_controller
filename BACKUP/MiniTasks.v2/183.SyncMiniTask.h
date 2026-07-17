
struct SyncMiniTask
  : TimeoutMiniTask
  , UartMiniTask
{
  void init(
      MiniTaskObserver miniTaskObserver,
      uint32_t timeout_time_ms,
      UartDevice * uartDevice)
  {
    TimeoutMiniTask::init(miniTaskObserver, timeout_time_ms);
    UartMiniTask::init(uartDevice);
  }

  void discard_input()
  {
    while (availableForRead()) {
      read_byte(0);
    }
  }

  void on_loop(uint32_t now_ms)
  {
    discard_input();
    TimeoutMiniTask::on_loop(now_ms);
  }

  static void on_loop_proxy(SyncMiniTask * syncMiniTask, uint32_t now_ms)
  {
    syncMiniTask->on_loop(now_ms);
  }
};
