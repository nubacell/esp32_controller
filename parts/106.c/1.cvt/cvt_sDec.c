
size_t
cvt_sDec(char* buffer, int64_t number)
{
  if (number < 0) {
    *buffer++ = '-';
    number = -number;
  }
  return cvt_ud2a(buffer, number, 10U, false);
}
