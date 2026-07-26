
// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_ud2a(char* buffer, uint64_t number, uint8_t base, bool upperCase)
{
  if (!number)
  {
    *buffer = '0';
    return 1;
  }

  uint8_t digits[CVT_BINARY_MAX_DIGITS];
  size_t digitIndex = 0;

  while (number)
  {
    digits[digitIndex++] = number % base;
    number /= base;
  }
  size_t size = digitIndex;
  while (digitIndex)
  {
    uint8_t ASCIIdigit = digits[--digitIndex];
    if (ASCIIdigit < 10)
    {
      ASCIIdigit += '0';
    }
    else
    {
      ASCIIdigit += upperCase ? ('A' - 10) : ('a' - 10);
    }
    *buffer++ = ASCIIdigit;
  }
  return size;
}
