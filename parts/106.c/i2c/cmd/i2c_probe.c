
void i2c_probe(const char * args)
{
  unsigned device = command_args_to_ul(args, nullptr);
  LOG("device=0x%X/%u", device, device);
  if (!::nc::i2c::bus.init_done_) {
    if (!::nc::i2c::bus.init()) { return; }
  }
  const auto probe_r = ::nc::i2c::bus.probe(device);
  LOG("probe_r=%i", probe_r);
}
