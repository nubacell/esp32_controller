
size_t UartDevice::availableForWrite() const
{
  const auto available = uartConfig_->availableForWrite();
  if (stats_.availableForWrite_max_ < available) {
    stats_.availableForWrite_max_ = available;
  }
  if (stats_.availableForWrite_min_ > available) {
    stats_.availableForWrite_min_ = available;
  }
  return available;
}
