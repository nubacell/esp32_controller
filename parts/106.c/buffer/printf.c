
namespace nc {
namespace mk {

size_t Buffer::printf(const char* format, ...) noexcept
{
  va_list argsList;
  va_start(argsList, format);
  size_t used = vprintf(format, argsList);
  va_end(argsList);
  return used;
}

} // namespace mk
} // namespace nc
