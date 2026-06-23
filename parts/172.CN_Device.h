
#define cnLOG(format, args...) LOG("%s| " format, uartConfig_->tag, ##args)
struct CN_Device
  : UartDevice
{
  CN_Device(
      const UartConfig * uartConfig)
    : UartDevice(uartConfig)
  {}

  void on_loop(uint32_t now_ms) override
  {
    // FIXME: implement necessary 'business logic'
    UartDevice::on_loop(now_ms);
  }
};
