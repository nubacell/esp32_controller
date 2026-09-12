
const char * skip_whitespace(const char * p)
{
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  if (!p) { return p; }

  while (' ' == *p || '\t' == *p) {
    ++p;
    //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  }
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  return p;
}

char * skip_whitespace(char * p)
{
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  if (!p) { return p; }

  while (' ' == *p || '\t' == *p) {
    ++p;
    //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  }
  //LOG("p=%p 0x%02X", p, (p) ? *p : 0);
  return p;
}
