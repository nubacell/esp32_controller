
#define cnLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct CN_Device
  : Runnable
{
  const UartConfig * uartConfig_;

  static constexpr size_t buffer_size = 64;
  byte buffer[buffer_size];
  LineReader lineReader_;

  void lineReader_on_eol(cb_arg_t cb_arg)
  {
    cnLOG("buffer='%s'", buffer);
  }
  static void lineReader_on_eol_proxy(cb_ctx_t cb_ctx, cb_arg_t cb_arg)
  {
    CN_Device * cnDevice = (CN_Device*) cb_ctx;
    cnDevice->lineReader_on_eol(cb_arg);
  }

  void lineReader_on_overflow(cb_arg_t cb_arg)
  {
    cnLOG("buffer='%s'", buffer);
  }
  static void lineReader_on_overflow_proxy(cb_ctx_t cb_ctx, cb_arg_t cb_arg)
  {
    CN_Device * cnDevice = (CN_Device*) cb_ctx;
    cnDevice->lineReader_on_overflow(cb_arg);
  }

  CN_Device(
      const UartConfig * uartConfig)
    : uartConfig_(uartConfig)
    , lineReader_(uartConfig,
        Callback(lineReader_on_eol_proxy, this),
        Callback(lineReader_on_overflow_proxy, this),
        buffer, sizeof(buffer))
  {}

  void on_loop(uint32_t now_ms) override
  {
    lineReader_.on_loop(now_ms);
  }
};
