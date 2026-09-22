
// 6.144 / 32768. = 0.0001875
// const auto volts = double(v) * 6.144 / 32768.;

::nc::mk::Buffer &
I_Meter::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << status
    << ' '
    << value * 1875;
}
