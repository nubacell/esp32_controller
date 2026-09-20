
namespace nc {
namespace stats {

struct OkNg
{
  counter_t counter_[2];

  static constexpr const char * const tags[] = {
    "ok",
    "ng",
  };

  // 0 OK
  // 1 NG
  inline
  size_t index_(bool ok) const noexcept
  {
    return !ok;
  }

  inline
  const counter_t & counter(bool ok) const noexcept
  {
    return counter_[index_(ok)];
  }

  inline
  counter_t & counter(bool ok) noexcept
  {
    return counter_[index_(ok)];
  }

  inline
  const counter_t & ok() const noexcept
  {
    return counter(true);
  }

  inline
  counter_t & ok() noexcept
  {
    return counter(true);
  }

  inline
  const counter_t & ng() const noexcept
  {
    return counter(false);
  }

  inline
  counter_t & ng() noexcept
  {
    return counter(false);
  }

  inline
  const counter_t & operator[](bool ok) const noexcept
  {
    return counter(ok);
  }

  inline
  counter_t & operator[](bool ok) noexcept
  {
    return counter(ok);
  }

  inline
  void inc(bool ok) noexcept
  {
    ++counter(ok);
  }

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace stats
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::stats::OkNg & okNg) noexcept
{
  return okNg.to_buffer(b);
}
