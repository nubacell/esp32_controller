
#define I_Meter_Stats_STRLEN ( \
  CONSTSTRLEN("I_Meter/stats") \
  + ( \
    + CONSTSTRLEN("  0:") \
    + CONSTSTRLEN(" start=1234567890/1234567890") \
    + CONSTSTRLEN(" poll=1234567890/1234567890") \
    + CONSTSTRLEN(" read=1234567890/1234567890") \
  ) * I_METER_CHANNELS_COUNT \
  + CONSTSTRLEN("\n") \
)

void I_Meter::reportStats(::nc::mk::Buffer & b) const
{
  for (unsigned i = 0; i < I_METER_CHANNELS_COUNT; ++i) {
    const auto & stats = stats_[i];
    b.printf(
        "  %u:"
        " start=%u/%u"
        " poll=%u/%u"
        " read=%u/%u",
        i,
        stats.start_, stats.start_timeout_,
        stats.poll_, stats.poll_timeout_,
        stats.read_, stats.read_timeout_);
  }
}

void I_Meter::reportStats() const
{
  char buffer[I_Meter_Stats_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);
  b.printf("I_Meter/stats");
  reportStats(b);
  b.printf("\n");
  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);
}
