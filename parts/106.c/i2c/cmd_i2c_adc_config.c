
void i2c_adc_config(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  ::nc::i2c::adc_regs::dump( *(::nc::i2c::adc_regs::Config*) &v);
}
