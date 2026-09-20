
::nc::mk::Buffer &
::nc::lgt::to_buffer(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Voltages & voltages)
{
  for (unsigned i = 0; i < ::nc::lgt::voltage_pins_COUNT; ++i) {
    b << ' '
      << voltages[i];
  }
  return b;
}
