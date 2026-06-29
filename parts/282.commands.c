
// u1_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u1_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u1_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u1_lf *IDN?

// u2_hex 0x0D 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0D 0x0A
// u2_hex 0x0A 0x0D 0x2A 0x49 0x44 0x4E 0x3F 0x0A 0x0D
// u2_hex 0x0A 0x2A 0x49 0x44 0x4E 0x3F 0x0A
// u2_lf *IDN?
// u2_lfcr *IDN?
// u2_cr *IDN?
// u2_crlf *IDN?
// u2_lf *CLS?

//static
void ux_hex(unsigned n, const char * args)
{
  const UartConfig * uartConfig0 = xx_uartConfigs[0];
  const UartConfig * uartConfig = xx_uartConfigs[n];
  for (;;) {
    args = skip_whitespace(args);
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

void ux_crlf(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\r');
  uartConfig->write('\n');
}

//static
void ux_lf(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\n');
}

void ux_lfcr(unsigned n, const char * args)
{
  const UartConfig * uartConfig = xx_uartConfigs[n];
  uartConfig->write(args);
  uartConfig->write('\n');
  uartConfig->write('\r');
}

void u1_hex(const char * args) { ux_hex(1, args); }
void u2_hex(const char * args) { ux_hex(2, args); }
void u1_cr(const char * args) { ux_cr(1, args); }
void u2_cr(const char * args) { ux_cr(2, args); }
void u1_crlf(const char * args) { ux_crlf(1, args); }
void u2_crlf(const char * args) { ux_crlf(2, args); }
void u1_lf(const char * args) { ux_lf(1, args); }
void u2_lf(const char * args) { ux_lf(2, args); }
void u1_lfcr(const char * args) { ux_lfcr(1, args); }
void u2_lfcr(const char * args) { ux_lfcr(2, args); }
