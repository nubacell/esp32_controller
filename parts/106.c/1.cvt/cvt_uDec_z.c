
size_t
cvt_uDec_z(char* buffer, uint64_t number)
{
  size_t size = cvt_uDec(buffer, number);
  buffer[size] = '\0';
  return size;
}
