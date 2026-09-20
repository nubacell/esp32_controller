
namespace nc {
namespace cli {

void spi_dump(const char * args)
{
  (void) args;
  spi_report_stats();
  spi_report_data();
}

} // namespace cli
} // namespace nc
