
namespace nc {
namespace mk {

size_t Buffer::put_char(char c) noexcept
{
  if (size_ > used_) {
    char * data = &data_[used_];
    size_t free = size_ - used_;
    if (free > 1) {
      data[0] = c;
      data[1] = '\0';
    } else {
      data[0] = '\0';
    }
  }
  size_t used = 1;
  used_ += used;
  return used;
}

} // namespace mk
} // namespace nc
