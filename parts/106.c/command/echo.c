
void command_echo(const char * args)
{
  (void) args;

  auto & buffer = ::nc::shared_buffers.echo_buffer;
  size_t buffer_size = sizeof(buffer);
  ::nc::mk::Buffer b(buffer, buffer_size);

  b.printf("/ECHO '%s'\n", args);

  const auto need = b.need();
  SERIAL_WRITE_BUFFER(buffer, buffer_size, need);

  uart_stats_report();
}
