
namespace nc {

typedef union shared_buffers_u
{
  char echo_buffer[echo_buffer_size];
  char uart_stats_buffer[uart_stats_buffer_size];
} shared_buffers_t;

extern shared_buffers_t shared_buffers;

} // namespace nc
