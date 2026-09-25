
void i2c_adc_read_repeat(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read_repeat(v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}
