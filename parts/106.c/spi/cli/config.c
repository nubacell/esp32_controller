
namespace nc {
namespace cli {

void spi_config(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    const uint32_t v = command_args_to_u32(args, nullptr);
    ::nc::spi::sensors.offline_config(v);
  }
  ::nc::spi::sensors.offline_config_dump();
}

} // namespace cli
} // namespace nc
