
void scope_sync_setup()
{
  LOG("");
#if 1
  gpio_set_level(gpio_num_t(SCOPE_SYNC), 1);
  gpio_set_direction(gpio_num_t(SCOPE_SYNC), GPIO_MODE_INPUT_OUTPUT);
#else
  digitalWrite(SCOPE_SYNC, HIGH);
  pinMode(SCOPE_SYNC, OUTPUT);
#endif
}

void scope_sync_on(bool on)
{
#if 1
  gpio_set_level(gpio_num_t(SCOPE_SYNC), !on);
#else
  digitalWrite(SCOPE_SYNC, (on) ? LOW : HIGH);
#endif
}

void scope_sync_pulse(unsigned us)
{
  scope_sync_on(true);
  if (us) { const auto us0 = esp_timer_get_time(); while ((esp_timer_get_time() - us0) < us) {} }
  scope_sync_on(false);
}

void scope_sync_on(const char * args)
{
  (void) args;
  LOG("");
  scope_sync_on(true);
}

void scope_sync_off(const char * args)
{
  (void) args;
  LOG("");
  scope_sync_on(false);
}

void scope_sync_pulse(const char * args)
{
  unsigned us = command_args_to_ul(args, nullptr);
  LOG("us=%u", us);
  scope_sync_pulse(us);
}
