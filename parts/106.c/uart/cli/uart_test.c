
namespace nc {
namespace cli {

void uart_test(const char * args)
{
  const char * tail = args;
  //LOG("tail=%p/'%s'", tail, tail ? tail : "");
  //cnDevice.uartConfig_->finish_output();
  unsigned count = command_args_to_u32(tail, &tail);
  unsigned width = command_args_to_u32(tail, nullptr);
  LOG("count=%u width=%u", count, width);
  //cnDevice.uartConfig_->finish_output();

  uint64_t time_12_us_min = uint64_t(-1);
  uint64_t time_23_us_min = uint64_t(-1);
  uint64_t time_13_us_min = uint64_t(-1);

  uint64_t time_12_us_max = 0;
  uint64_t time_23_us_max = 0;
  uint64_t time_13_us_max = 0;

  uint64_t time_0_us = esp_timer_get_time();
  static constexpr uint64_t el_period_us = 600000;
  uint64_t el_time_us = time_0_us + el_period_us;

  for (unsigned step = 0; step < count; ++step) {
    uint64_t time_1_us = esp_timer_get_time();

    auto & buffer = ::nc::shared_buffers.uart_test_buffer;
    size_t buffer_size = sizeof(buffer);
    ::nc::mk::Buffer b(buffer, buffer_size);

    const auto time_us = time_1_us - time_0_us;
    b.printf("/UART/TEST %5u %6" PRIu64 ".%06u ", step, time_us / 1000000, unsigned(time_us % 1000000));

    static const char chars[] =
        ".0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (unsigned i = 0; i < width; ++i) {
#if 1
      b.put_char(chars[i % (sizeof(chars) - 1)]);
#else
      b.printf("%c", chars[i % (sizeof(chars) - 1)]);
#endif
    }

    b.printf("|");
    b.printf("\n");
    uint64_t time_2_us = esp_timer_get_time();

    size_t tx_available_1 = cnDevice.availableForWrite();
    SERIAL_WRITE_BUFFER(buffer, buffer_size, b.need());
    size_t tx_available_2 = cnDevice.availableForWrite();
    //cnDevice.uartConfig_->finish_output();

    uint64_t time_3_us = esp_timer_get_time();

    LOG("tx_available_1=%zu tx_available_2=%zu", tx_available_1, tx_available_2);

    uint64_t time_12_us = time_2_us - time_1_us;
    uint64_t time_23_us = time_3_us - time_2_us;
    uint64_t time_13_us = time_3_us - time_1_us;
    LOG("time %" PRIu64 " %" PRIu64 " %" PRIu64, time_12_us, time_23_us, time_13_us);
    //cnDevice.uartConfig_->finish_output();

    if (time_12_us_min > time_12_us) { time_12_us_min = time_12_us; }
    if (time_23_us_min > time_23_us) { time_23_us_min = time_23_us; }
    if (time_13_us_min > time_13_us) { time_13_us_min = time_13_us; }

    if (time_12_us_max < time_12_us) { time_12_us_max = time_12_us; }
    if (time_23_us_max < time_23_us) { time_23_us_max = time_23_us; }
    if (time_13_us_max < time_13_us) { time_13_us_max = time_13_us; }

    if (el_time_us < time_3_us) {
      el_id(nullptr);
      el_time_us = time_3_us + el_period_us;
    }
    elDevice.on_loop(time_3_us / 1000);
  }
  LOG("time min %" PRIi64 " %" PRIi64 " %" PRIi64, time_12_us_min, time_23_us_min, time_13_us_min);
  LOG("time max %" PRIi64 " %" PRIi64 " %" PRIi64, time_12_us_max, time_23_us_max, time_13_us_max);

  uart_stats_report();
}

} // namespace cli
} // namespace nc
