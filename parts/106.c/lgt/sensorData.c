
::nc::mk::Buffer &
::nc::lgt::SensorData::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    //<< ' '
    //<< device_id
    << ' '
    << unsigned(vcc) * 125
    //<< ' '
    << analogSignals;
}
