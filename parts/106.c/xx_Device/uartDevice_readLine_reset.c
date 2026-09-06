
// assuming 'sizeof(buffer_) > 0'
void UartDevice::on_loop_readLine_reset()
{
  buffer_used_ = 0;
  overflow_ = false;
  eol_ = false;
  buffer_[0] = '\0'; // trailing '\0' for convenience
}
