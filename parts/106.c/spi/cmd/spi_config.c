
void spi_config(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    const uint32_t v = command_args_to_u32(args, nullptr);
    ::nc::spi::sensors::config(v);
  }
  ::nc::spi::sensors::config_dump();
}
