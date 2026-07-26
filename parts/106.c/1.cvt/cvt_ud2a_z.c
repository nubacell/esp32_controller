
// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_ud2a_z(char* buffer, uint64_t number, uint8_t base, bool upperCase)
{
  size_t size = cvt_ud2a(buffer, number, base, upperCase);
  buffer[size] = '\0';
  return size;
}
