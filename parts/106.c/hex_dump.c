
static char nibble_hex(uint8_t b)
{
  uint8_t n = b & 0x0F;
  return (n < 10) ? '0' + n : 'A' - 10 + n;
}

static void hex_dump_byte(char * p, uint8_t b)
{
  p[0] = nibble_hex(b >> 4);
  p[1] = nibble_hex(b);
}

static char make_printable(uint8_t b)
{
  b &= 0x7F;
  return (b < 0x20 || b >= 0x7F) ? '.' : b;
}

void hex_dump_line(
    char * hex_s, size_t hex_s_size,
    char * char_s, size_t char_s_size,
    const uint8_t * data, size_t size)
{
  memset(hex_s, ' ', hex_s_size - 1);
  hex_s[hex_s_size - 1] = '\0';
  memset(char_s, ' ', char_s_size - 1);
  char_s[char_s_size - 1] = '\0';
  for (unsigned gi = 0; gi < 4; ++gi) {
    for(unsigned bi = 0; bi < 4; ++bi) {
      unsigned di = gi * 4 + bi;
      unsigned hex_s_i = gi * (2 + 4*2+3) + bi * (2+1);
      unsigned char_s_i = gi * (1 + 4) + bi;
      if (di < size) {
        uint8_t d = data[di];
        hex_dump_byte(&hex_s[hex_s_i], d);
        char_s[char_s_i] = make_printable(d);
      } else {
        hex_s[hex_s_i  ] = '.';
        hex_s[hex_s_i+1] = '.';
        char_s[char_s_i] = '.';
      }
    }
  }
}

// "XXXXXXXX/XXX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XXXX XXXX XXXX XXXX"
void hex_dump(const uint8_t * data, size_t size)
{
  //LOG("data=%p size=%zu", data, size);
  static
  char s[sizeof("XXXXXXXX/XXX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX  XXXX XXXX XXXX XXXX")];
  for (uint32_t offs = 0; offs < size; offs += 16) {
    const uint8_t * addr = data + offs;
    static
    char hex_s[sizeof("XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX")];
    static
    char char_s[sizeof("XXXX XXXX XXXX XXXX")];
    size_t left = size - offs;
    hex_dump_line(hex_s, sizeof(hex_s), char_s, sizeof(char_s), addr, left);
    //LOG("hex_s='%s'", hex_s);
    //LOG("char_s='%s'", char_s);
    snprintf(s, sizeof(s), "%08X/%03X  %s  %s", addr, offs, hex_s, char_s);
    Serial.print(". . ");
    Serial.println(s);
  }
}
