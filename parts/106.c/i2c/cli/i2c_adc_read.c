
void i2c_adc_read(const char * args)
{
  unsigned register_number = command_args_to_u32(args, nullptr);
  LOG("register_number=0x%X/%u", register_number, register_number);
  if (register_number >= 4) {
    LOG("ERROR: register_number must be < 4");
    return;
  }
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register(register_number), v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}
