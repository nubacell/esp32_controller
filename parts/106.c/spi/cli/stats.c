
namespace nc {
namespace cli {

void spi_stats(const char * args)
{
  (void) args;
  ::nc::spi::stats::report();
}

} // namespace cli
} // namespace nc
