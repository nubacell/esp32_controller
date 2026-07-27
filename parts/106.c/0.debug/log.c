
void log(const char *file, unsigned line, const char * function, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);

  constexpr size_t buffer_size = 172;
  static char buffer[buffer_size];
  const int vsnprintf_r = vsnprintf(buffer, sizeof(buffer), format, argsList);
  log_write(line, function, buffer);

  va_end(argsList);

  if (vsnprintf_r < 0) {
    const int snprintf_r = snprintf(buffer, sizeof(buffer),
        "ALERT: '%s()' failure, errno=%i", "vsnprintf", errno);
    if (snprintf_r < 0) {
      log_write(__LINE__, __FUNCTION__, "ALERT: 'snprintf()' failure");
    } else {
      log_write(__LINE__, __FUNCTION__, buffer);
    }
  } else {
    const size_t output_length = (size_t) vsnprintf_r;
    const size_t needed_size = output_length + 1; // add space for terminating '\0'
    if (buffer_size < needed_size) {
      const int snprintf_r = snprintf(buffer, sizeof(buffer),
          "ALERT: logger buffer overflow (needed=%zu available=%zu)",
          needed_size, sizeof(buffer));
      if (snprintf_r < 0) {
        log_write(__LINE__, __FUNCTION__, "ALERT: 'snprintf()' failure");
      } else {
        log_write(__LINE__, __FUNCTION__, buffer);
      }
    }
  }
}
