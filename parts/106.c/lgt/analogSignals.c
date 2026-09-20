
::nc::mk::Buffer &
::nc::lgt::AnalogSignals::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  ::nc::lgt::to_buffer(b, voltages);
  ::nc::lgt::to_buffer(b, temperatures);
  return b;
}
