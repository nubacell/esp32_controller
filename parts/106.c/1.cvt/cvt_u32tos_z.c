
// Assuming 'base' is 2, 8, 10, or 16.
size_t
cvt_u32tos_z(char* buffer, uint32_t number, uint8_t base, bool upperCase)
{
  size_t size = cvt_u32tos(buffer, number, base, upperCase);
  buffer[size] = '\0';
  return size;
}
