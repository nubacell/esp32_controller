
// Config.OS == 0 -- ADC performs conversion
bool I_Meters::poll(bool & ready)
{
  ::nc::i2c::adc_regs::Config c = {};
  const auto r = ::nc::i2c::adc.read_repeat( *(uint16_t*) &c);
  ++stats.poll[r];
  ready = r && c.OS;
  return r;
}
