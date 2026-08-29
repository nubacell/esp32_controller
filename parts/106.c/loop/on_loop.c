
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
