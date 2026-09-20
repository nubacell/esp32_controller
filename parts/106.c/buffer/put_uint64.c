
namespace nc {
namespace mk {

size_t Buffer::put(uint64_t v) noexcept
{
  char buffer[sizeof(v)*8];
  const size_t size = ::nc::mk::cvt::uint64_to_ascii(buffer, sizeof(buffer), v, 10, false);
  return put(buffer, size);
}

} // namespace mk
} // namespace nc
