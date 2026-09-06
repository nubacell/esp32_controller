
// TODO: Implement 'runnables_[]' switching

static Runnable * const runnables_[] = {
  &::nc::loop::stats_,
  &cnDevice,
  &psDevice,
  &elDevice,
  &iMeter,
  &spiDevice,
  &test,
  &::nc::monitor_,
};

void loop()
{
  uint32_t now_ms = millis();

  for (unsigned i = 0; i < ARRAY_SIZE(runnables_); ++i) {
    runnables_[i]->on_loop(now_ms);
  }
}
