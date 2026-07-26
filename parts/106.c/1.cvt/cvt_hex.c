
size_t
cvt_hex(char* buffer, uint64_t number, bool upperCase)
{
  return cvt_ud2a(buffer, number, 16U, upperCase);
}
