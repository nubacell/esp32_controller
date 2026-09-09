
namespace nc {
namespace cli {

void uart_stats(const char * args)
{
  (void) args;
  uart_stats_report();
}

} // namespace cli
} // namespace nc
