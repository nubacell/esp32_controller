
void log(const char *file, unsigned line, const char * function, const char * format, ...)
{
  va_list argsList;
  va_start(argsList, format);

  constexpr size_t buffer_size = 100;
  char buffer[buffer_size];
  size_t output_length = vsnprintf(buffer, sizeof(buffer), format, argsList);
  log_write(line, function, buffer);

  va_end(argsList);

  size_t needed_size = output_length + 1; // add space for terminating '\0'
  if (buffer_size < needed_size) {
    snprintf(buffer, sizeof(buffer),
        "ALERT: logger buffer overflow (needed=%zu available=%zu)",
        needed_size, sizeof(buffer));
    log_write(__LINE__, __FUNCTION__, buffer);
  }
}
