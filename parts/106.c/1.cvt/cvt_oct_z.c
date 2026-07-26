
size_t
cvt_oct_z(char* buffer, uint64_t number)
{
  size_t size = cvt_oct(buffer, number);
  buffer[size] = '\0';
  return size;
}
