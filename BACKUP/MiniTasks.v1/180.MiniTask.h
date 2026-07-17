
struct MiniTask;

struct MiniTaskObserver
{
  void onTaskFinished(MiniTask * miniTask);
};

struct MiniTask
{
  MiniTaskObserver * miniTaskObserver_;

  MiniTask(MiniTaskObserver * miniTaskObserver)
    : miniTaskObserver_(miniTaskObserver)
  {}

  void onTaskFinished()
  {
    miniTaskObserver_->onTaskFinished(this);
  }

  virtual
  void on_loop(uint32_t now_ms)
  {
    onTaskFinished();
  }
};
