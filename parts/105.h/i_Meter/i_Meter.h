
struct I_Meter
{
  struct Status
  {
    bool adc_done;

    inline void reset() { adc_done = false; }

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } status;

  uint16_t value; // conversion_result;
  void update_value();

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meter::Status & status) noexcept
{
  return status.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const I_Meter & iMeter) noexcept
{
  return iMeter.to_buffer(b);
}
