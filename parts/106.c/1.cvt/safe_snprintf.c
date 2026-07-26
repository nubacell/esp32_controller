
size_t safe_vsnprintf(char * buffer, size_t size, const char * format, va_list ap)
{
  const auto r = vsnprintf(buffer, size, format, ap);
  return (r < 0) ? 0 : r;
}

size_t safe_snprintf(char * buffer, size_t size, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);
  size_t r = safe_vsnprintf(buffer, size, format, argsList);
  va_end(argsList);
  return r;
}
