
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
