
struct MiniTask;

using MiniTaskObserver = Callback;

struct MiniTask
{
  MiniTaskObserver miniTaskObserver_;

  void init(MiniTaskObserver miniTaskObserver)
  {
    miniTaskObserver_ = miniTaskObserver;
  }

  void onTaskFinished()
  {
    miniTaskObserver_(this);
  }

  void on_loop(uint32_t now_ms)
  {
    onTaskFinished();
  }

  static void on_loop_proxy(MiniTask * miniTask, uint32_t now_ms)
  {
    miniTask->on_loop(now_ms);
  }
};
