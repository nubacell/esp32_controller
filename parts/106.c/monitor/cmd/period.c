
using namespace nc;

void monitor_period(const char * args)
{
  monitor_.pause_.period_ms_ = command_args_to_u32(args, nullptr);
  LOG("monitor_.pause_.period_ms_=%u", monitor_.pause_.period_ms_);
}
