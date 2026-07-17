
void ux_baud(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];

  char * endptr = NULL;
  uint32_t baudrate = strtoul(args, &endptr, 0);
  if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }
  LOG("setting baudrate=%u", baudrate);
  uartConfig->baudRate(baudrate);
  LOG("active baudrate=%u", uartConfig->baudRate());
}

void u1_baud(const char * args) { ux_baud(1, args); }
void u2_baud(const char * args) { ux_baud(2, args); }
