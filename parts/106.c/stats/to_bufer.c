
namespace nc {
namespace stats {

::nc::mk::Buffer &
OkNg::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << ok()
    << ' '
    << ng();
}

} // namespace stats
} // namespace nc
