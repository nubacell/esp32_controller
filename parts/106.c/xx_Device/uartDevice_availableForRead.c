
size_t UartDevice::availableForRead() const
{
  const auto available = uartConfig_->availableForRead();
  if (stats_.availableForRead_max_ < available) {
    stats_.availableForRead_max_ = available;
  }
  return available;
}
