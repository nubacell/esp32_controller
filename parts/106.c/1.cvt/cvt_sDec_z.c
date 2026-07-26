
size_t
cvt_sDec_z(char* buffer, int64_t number)
{
  size_t size = cvt_sDec(buffer, number);
  buffer[size] = '\0';
  return size;
}
