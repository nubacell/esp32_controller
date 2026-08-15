
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

size_t I_Meter::reportStats(char * buffer, size_t buffer_size) const
{
  size_t used = safe_snprintf(buffer, buffer_size, "I_Meter/stats");
  for (unsigned i = 0; i < I_METER_CHANNELS_COUNT && buffer_size > used; ++i) {
    const size_t left = buffer_size - used;
    const auto & stats = stats_[i];
    used += safe_snprintf(&buffer[used], left,
        "  %u:"
        " start=%u/%u"
        " poll=%u/%u"
        " read=%u/%u",
        i,
        stats.start_, stats.start_timeout_,
        stats.poll_, stats.poll_timeout_,
        stats.read_, stats.read_timeout_);
  }
  if (buffer_size > used) {
    const size_t left = buffer_size - used;
    used += safe_snprintf(&buffer[used], left, "\n");
  }
  return used;
}

void I_Meter::reportStats() const
{
  char buffer[I_Meter_Stats_STRLEN+1];
  size_t buffer_size = sizeof(buffer);
  const size_t used = reportStats(buffer, buffer_size);
  SERIAL_WRITE_BUFFER(buffer, buffer_size, used);
}
