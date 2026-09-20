
namespace nc {
namespace mk {

size_t Buffer::put(int64_t v) noexcept
{
  uint64_t u;
  if (v < 0) {
    u = -v;
    put_char('-');
  } else {
    u = v;
  }
  return put(u);
}

} // namespace mk
} // namespace nc
