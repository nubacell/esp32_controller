
void stats(const char * args)
{
  (void) args;
  uart_stats_report();
  ::nc::i2c::stats::report();
  ::nc::spi::stats::report();
}
