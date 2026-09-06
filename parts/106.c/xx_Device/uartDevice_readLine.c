
// returns '(eol_ || overflow_)'
bool UartDevice::on_loop_readLine()
{
  if (!eol_ && !overflow_) {
    auto available = availableForRead();
    while (available--) {
      const auto b = uartConfig_->read_byte(0);
#if 1
      if ('\r' == last_char_ && '\n' == b) { // CR+LF
        last_char_ = 0; // intentionally store not '\n'
      } else
      if ('\n' == last_char_ && '\r' == b) { // LF+CR
        last_char_ = 0; // intentionally store not '\r'
      } else
#endif
      if ('\n' == b || '\r' == b) { // CR || LF
        eol_ = true;
        // last_char_ will be used on next byte to detect CR+LF and LF+CR
        last_char_ = b;
        break;
      } else
      // reserve space for trailing '\0'
      if ((buffer_used_ + 1) < sizeof(buffer_)) {
        buffer_[buffer_used_++] = b;
        buffer_[buffer_used_] = '\0';
        last_char_ = b;
      } else {
        overflow_ = true;
        last_char_ = b;
        break;
      }
    }
  }
  return eol_ || overflow_;
}
