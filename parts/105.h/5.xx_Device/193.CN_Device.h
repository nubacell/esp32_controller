
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
    size_t len;
    const char * args;
    if (!p) {
      len = strlen(s);
      args = s + len;
    } else {
      len = p - s;
      args = p + 1;
    }
    cnLOG("len=%zu args='%s'", len, args);

    command_fn_t command_fn = command_s_to_fn(s, len);

    if (!command_fn) {
      cnLOG("There is no command '%*s'", (int)len, s);
      print_commands();
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

extern CN_Device cnDevice;
