
bool I_Meter::read(uint16_t & v)
{
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
  if (!r) {
    ++stats_[channel_].read_timeout_;
  } else {
    ++stats_[channel_].read_;
  }
  return r;
}
