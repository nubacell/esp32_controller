
::nc::mk::Buffer &
I_Meter::Status::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << uint8_t(!transfer_done);
}
