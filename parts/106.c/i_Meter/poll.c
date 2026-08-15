
// Config.OS == 0 -- ADC performs conversion
bool I_Meter::poll(bool & ready)
{
  ::nc::i2c::adc_regs::Config c = {};
  const auto r = ::nc::i2c::adc.read_repeat( *(uint16_t*) &c);
  if (!r) {
    ++stats_[channel_].poll_timeout_;
  } else {
    ready = c.OS;
    ++stats_[channel_].poll_;
  }
  return r;
}
