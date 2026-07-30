
void serial_write_buffer(
    const char *file, unsigned line, const char * function,
    char * buffer, size_t buffer_size, size_t used);

#define SERIAL_WRITE_BUFFER(buffer, buffer_size, used) \
    serial_write_buffer( \
        __FILE__, __LINE__, __PRETTY_FUNCTION__, \
        buffer, buffer_size, used);
