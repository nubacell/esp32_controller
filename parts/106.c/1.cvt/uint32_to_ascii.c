
namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint32_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint32_t number,
    uint8_t base,
    bool upperCase)
{
  size_t buffer_index = 0;
  if (!number)
  {
    if (buffer_index < buffer_size) { buffer[buffer_index] = '0'; }
    ++buffer_index;
  }
  else
  {
    uint8_t digits[sizeof(number)*8];
    size_t digitIndex = 0;
    while (number)
    {
      digits[digitIndex++] = number % base;
      number /= base;
    }
    while (digitIndex)
    {
      uint8_t digit = digits[--digitIndex];
      digit += (digit < 10) ? '0' : upperCase ? ('A' - 10) : ('a' - 10);
      if (buffer_index < buffer_size) { buffer[buffer_index] = digit;}
      ++buffer_index;
    }
  }
  return buffer_index;
}

} // namespace cvt
} // namespace mk
} // namespace nc
