
// u1_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u1_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u1_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u1_lf *IDN?

// u2_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u2_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u2_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u2_lf *IDN?

  //static
  void ux_hex(unsigned n, const char * args)
  {
    const UartConfig * uartConfig0 = xx_uartConfigs[0];
    const UartConfig * uartConfig = xx_uartConfigs[n];
    for (;;) {
      while (' ' == *args) { ++args; }
      if (!*args) { break; }
      char * endptr = NULL;
      uint8_t xx = strtoul(args, &endptr, 0);
      LOG(". xx='0x%02X'", xx);

      uartConfig0->write('"');
      uartConfig0->write(xx);
      uartConfig0->write('"');
      uartConfig0->write('\r');
      uartConfig0->write('\n');

      uartConfig->write(xx);
      args = endptr;
    }
  }

  //static
  void ux_cr(unsigned n, const char * args)
  {
    const UartConfig * uartConfig = xx_uartConfigs[n];
    uartConfig->write(args);
    uartConfig->write('\r');
  }

  //static
  void ux_lf(unsigned n, const char * args)
  {
    const UartConfig * uartConfig = xx_uartConfigs[n];
    uartConfig->write(args);
    uartConfig->write('\r');
  }

  void u1_hex(const char * args) { ux_hex(1, args); }
  void u2_hex(const char * args) { ux_hex(2, args); }
  void u1_cr(const char * args) { ux_cr(1, args); }
  void u2_cr(const char * args) { ux_cr(2, args); }
  void u1_lf(const char * args) { ux_lf(1, args); }
  void u2_lf(const char * args) { ux_lf(2, args); }

  typedef void (*command_fn_t)(const char * args);

  typedef struct commands_map_s {
    const char * command_s;
    size_t command_len;
    command_fn_t command_fn;
  } commands_map_t;

  static const commands_map_t commands_map[] = {
#define CMD(x) { #x, CONSTSTRLEN(#x), x, }
    CMD(u1_hex),
    CMD(u2_hex),
    CMD(u1_cr),
    CMD(u2_cr),
    CMD(u1_lf),
    CMD(u2_lf),
#undef CMD
  };

  static
  void
  print_commands(
      const commands_map_t * commands_map,
      size_t commands_count)
  {
    LOG("Available commands:");
    for (unsigned i = 0; i < commands_count; ++i) {
      LOG(". %s", commands_map[i].command_s);
    }
  }

  static
  command_fn_t
  command_s_to_fn(
      const commands_map_t * commands_map,
      size_t commands_count,
      const char * command_s,
      size_t command_len)
  {
    for (unsigned i = 0; i < commands_count; ++i) {
      if (commands_map[i].command_len == command_len
      && !strncmp(command_s, commands_map[i].command_s, command_len)) {
        return commands_map[i].command_fn;
      }
    }
    return NULL;
  }

#define cnLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct CN_Device
  : UartDevice
{
  CN_Device(
      const UartConfig * uartConfig)
    : UartDevice(uartConfig)
  {
    cnLOG("device_tag='%s'", device_tag);
  }

  void on_loop_on_line(const char * s)
  {
    const char * p = strchr(s, ' ');
    size_t len = (!p) ? strlen(s) : p - s;
    const char * args = s + len + 1;
    cnLOG("len=%zu args='%s'", len, args);

    command_fn_t command_fn = command_s_to_fn(
        commands_map, ARRAY_SIZE(commands_map), s, len);

    if (!command_fn) {
      printf("There is no command '%*s'\n", (int)len, s);
      print_commands(commands_map, ARRAY_SIZE(commands_map));
    } else {
      command_fn(args);
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    if (on_loop_readLine()) {
      cnLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
      on_loop_on_line( (const char *) buffer_);
      on_loop_readLine_reset();
    }
  }
};
