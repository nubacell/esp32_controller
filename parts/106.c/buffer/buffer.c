
namespace nc {
namespace mk {

size_t Buffer::vprintf(const char * format, va_list argsList) noexcept
{
  char * data;
  size_t free;
  char buffer[1];
  if (size_ > used_) {
    data = &data_[used_];
    free = size_ - used_;
  } else {
    data = buffer;
    free = sizeof(buffer);
  }
  size_t used = safe_vsnprintf(data, free, format, argsList);
  used_ += used;
  return used;
}

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
