
namespace nc {
namespace spi {

::nc::mk::Buffer &
Sensors::Stats::to_buffer(::nc::mk::Buffer & b) const noexcept
{
  return b
    << ' '
    << post_setup_cb_count
    << ' '
    << post_trans_cb_count;
}

::nc::mk::Buffer &
Sensors::stats_to_buffer(::nc::mk::Buffer & b) const noexcept
{
  stats.to_buffer(b);
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    //b.printf(" %u:", i);
    b << sensors_[i].raw.stats;
  }
  return b;
}

} // namespace spi
} // namespace nc
