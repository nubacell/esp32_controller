
void UartDevice::on_loop(uint32_t now_ms)
{
  if (on_loop_readLine()) {
    udLOG("eol=%i overflow=%i buffer='%s'", eol_, overflow_, buffer_);
    on_loop_readLine_reset();
  }
}
