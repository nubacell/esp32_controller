
namespace nc {

typedef union shared_buffers_u
{
  char echo_buffer[echo_buffer_size];
  char spi_data_buffer[spi_data_buffer_size];
  char spi_raw_data_buffer[spi_raw_data_buffer_size];
  char spi_stats_buffer[spi_stats_buffer_size];
  char uart_stats_buffer[uart_stats_buffer_size];
  char uart_test_buffer[uart_test_buffer_size];
} shared_buffers_t;

extern shared_buffers_t shared_buffers;

} // namespace nc
