
size_t
cvt_bin_z(char* buffer, uint64_t number)
{
  size_t size = cvt_bin(buffer, number);
  buffer[size] = '\0';
  return size;
}
