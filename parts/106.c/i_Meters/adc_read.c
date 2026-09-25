
bool I_Meters::adc_read(uint16_t & v)
{
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
  ++stats.read[r];
  return r;
}
