
namespace nc {
namespace mk {

size_t Buffer::put(const char * data, size_t size) noexcept
{
  for (size_t i = 0; i < size; ++i) {
    put_char(data[i]);
  }
  return size;
}

} // namespace mk
} // namespace nc
