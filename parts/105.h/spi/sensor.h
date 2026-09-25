
namespace nc {
namespace spi {

struct Sensor
{
  // FIXME: Looks like 'Stats' belongs to 'Raw'
  struct Raw
  {
    struct Stats
    {
      ::nc::stats::OkNg transfer_init;
      ::nc::stats::OkNg transfer_done;
      ::nc::stats::OkNg rx_bits;
      ::nc::stats::OkNg crc;

      static constexpr const char * const tags[] = {
        "transfer_init",
        "transfer_done",
        "rx_bits",
        "crc",
      };

      ::nc::mk::Buffer &
      to_buffer(::nc::mk::Buffer & b) const noexcept;
    } stats;

    uint8_t tx_buffer[SPI_BUFFER_SIZE];
    union
    {
      uint8_t rx_buffer[SPI_BUFFER_SIZE];
      ::nc::lgt::SensorData data;
    };

    struct Status
    {
      bool transfer_done;

      uint16_t rx_bits;
      bool crc_valid_;

      inline bool rx_bits_valid() const { return rx_bits >= SPI_RX_BITS_MIN; }
      inline bool is_crc_valid() const { return crc_valid_; }

      ::nc::mk::Buffer &
      to_buffer(::nc::mk::Buffer & b) const noexcept;
    } status;

    uint8_t crc() const;
    bool is_crc_valid_impl() const;
    void update_crc_valid();

    void reset();

    ::nc::mk::Buffer &
    to_buffer(::nc::mk::Buffer & b) const noexcept;
  } raw;

  ::nc::lgt::SensorData data;
  void update_data();

  ::nc::mk::Buffer &
  to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace spi
} // namespace nc

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Raw::Stats & stats) noexcept
{
  return stats.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Raw::Status & status) noexcept
{
  return status.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor::Raw & raw) noexcept
{
  return raw.to_buffer(b);
}

static
inline
::nc::mk::Buffer &
operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::spi::Sensor & sensor) noexcept
{
  return sensor.to_buffer(b);
}
