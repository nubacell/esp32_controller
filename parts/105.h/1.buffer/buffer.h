
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

  size_t put_char(char c) noexcept;
  size_t put(const char * data, size_t size) noexcept;
  size_t put_s(const char * s) noexcept;

  inline
  Buffer & operator<<(char c) noexcept
  {
    put_char(c);
    return *this;
  }

  inline
  Buffer & operator<<(const char * s) noexcept
  {
    put_s(s);
    return *this;
  }

  size_t put(uint64_t v) noexcept;
  size_t put(int64_t v) noexcept;
  size_t put(uint32_t v) noexcept;
  size_t put(int32_t v) noexcept;

  inline size_t put(uint16_t v) noexcept { return put(uint32_t(v)); }
  inline size_t put(int16_t v) noexcept { return put(int32_t(v)); }
  inline size_t put(uint8_t v) noexcept { return put(uint32_t(v)); }
  inline size_t put(int8_t v) noexcept { return put(int32_t(v)); }

  inline size_t put(unsigned v) noexcept { return put(uint32_t(v)); }
  inline size_t put(signed v) noexcept { return put(int32_t(v)); }

#if 1
  template <typename T>
  inline Buffer & operator<<(T v) noexcept { put(v); return *this; }
#else
  inline Buffer & operator<<(uint64_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint32_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint16_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(int16_t v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(uint8_t v) noexcept { put(v); return *this; }

  inline Buffer & operator<<(unsigned v) noexcept { put(v); return *this; }
  inline Buffer & operator<<(signed v) noexcept { put(v); return *this; }
#endif
};

} // namespace mk
} // namespace nc
