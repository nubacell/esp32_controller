
void serial_write_buffer(
    const char * file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used)
{
  if (used < buffer_size) {
    Serial.write(buffer, used);
#if 0
    log(file, line, function,
        "buffer_size=%zu required=%zu", buffer_size, used + 1);
#endif
  } else {
    if (buffer_size >= 2) {
      buffer[buffer_size - 2] = '\n';
      buffer[buffer_size - 1] = '\0';
      Serial.write(buffer, buffer_size - 1);
    }
    log(file, line, function,
        "ERROR: buffer overflow (buffer_size=%zu required=%zu)",
        buffer_size, used + 1);
  }
}
