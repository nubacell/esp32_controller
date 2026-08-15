
/*
    There is little dirty trick.

    Normally 'used <= size'. However 'used > size' is permitted and means
    [logical] 'overflow' with excessive data lost.
*/

namespace nc {
namespace mk {

struct Buffer
{
  char * data_;
  size_t size_;
  size_t used_;

  Buffer(char * data, size_t size) noexcept
    : data_(data)
    , size_(size)
    , used_(0)
  {
  }

  size_t used() const noexcept { return std::min(used_, size_); }
  size_t need() const noexcept { return used_; }

  __attribute__ ((format (printf, 1, 2)));
  size_t printf(const char * format, ...) noexcept;
  size_t vprintf(const char * format, va_list argsList) noexcept;
};

} // namespace mk
} // namespace nc
