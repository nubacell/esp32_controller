
bool I_Meters::adc_start()
{
  const auto r = ::nc::i2c::adc.start_single_conversion(channel_);
  ++stats.start[r];
  return r;
}
