
size_t
cvt_hex_z(char* buffer, uint64_t number, bool upperCase)
{
  size_t size = cvt_hex(buffer, number, upperCase);
  buffer[size] = '\0';
  return size;
}
