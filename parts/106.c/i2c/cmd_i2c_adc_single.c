
void i2c_adc_single(const char * args)
{
  unsigned channel = command_args_to_ul(args, nullptr);
  LOG("channel=%u", channel);
  if (channel >= 4) {
    LOG("ERROR: channel must be < 4");
    return;
  }

  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint64_t us1 = esp_timer_get_time();
  {
    const auto r = ::nc::i2c::adc.start_single_conversion(channel);
    //LOG("r=%i", r);
    if (!r) { return; }
  }
  uint64_t us2 = esp_timer_get_time();
  unsigned wait_count = 0;
  bool converting;
  do {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read_repeat(v);
    //LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
    if (!r) { return; }
    ++wait_count;
    if (wait_count > 100) {
      LOG("wait_count limit exceeded");
      break;
    }
    const auto c = *(::nc::i2c::adc_regs::Config*) &v;
    converting = !c.OS;
  } while (converting);
  uint64_t us3 = esp_timer_get_time();
#if 0
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }
#endif
  uint64_t us4;
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
    us4 = esp_timer_get_time();
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i '%s'", r, v, v, int16_t(v), "Conversion");
  }
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us2, us1, us2 - us1);
  LOG("%s us3=%" PRIu64 " - us2=%" PRIu64 " =%" PRIu64, "write_read", us3, us2, us3 - us2);
  LOG("%s us4=%" PRIu64 " - us3=%" PRIu64 " =%" PRIu64, "write_read", us4, us3, us4 - us3);
  LOG("%s us4=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us4, us1, us4 - us1);
  LOG("wait_count=%u", wait_count);
}
