
::nc::mk::Buffer &
I_Meters::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << start
    << poll
    << read;
}
