
bool I_Meter::start()
{
  const auto r = ::nc::i2c::adc.start_single_conversion(channel_);
  if (!r) {
    ++stats_[channel_].start_timeout_;
  } else {
    ++stats_[channel_].start_;
  }
  return r;
}
