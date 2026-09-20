
namespace nc {
namespace cli {

void spi_run(const char * args)
{
  (void) args;
  ::nc::spi::sensors.run();
}

} // namespace cli
} // namespace nc
