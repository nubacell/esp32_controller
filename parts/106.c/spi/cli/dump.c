
namespace nc {
namespace cli {

void spi_dump(const char * args)
{
  (void) args;
  ::nc::spi::stats::report();
  spi_report_data();
}

} // namespace cli
} // namespace nc
