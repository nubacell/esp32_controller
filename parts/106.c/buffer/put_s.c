
namespace nc {
namespace mk {

size_t Buffer::put_s(const char * s) noexcept
{
  if (!s) { s = "<nullptr>"; }
  size_t used = 0;
  for (char c = *s; c; ++s, c = *s) {
    put_char(c);
    ++used;
  }
  return used;
}

} // namespace mk
} // namespace nc
