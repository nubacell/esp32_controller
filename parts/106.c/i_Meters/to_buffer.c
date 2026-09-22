
::nc::mk::Buffer &
I_Meters::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  for (unsigned i = 0; i < I_METERS_COUNT; ++i) {
    b << iMeters_[i];
  }
  return b;
}
