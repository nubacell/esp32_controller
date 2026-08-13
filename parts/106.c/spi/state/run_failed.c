
/*
    SPI transfer can fail. It results in 'timeout'.
    It can be fixed by chip re-init.
*/
void SPI_Device::on_state_run_failed(uint32_t now_ms)
{
  (void) now_ms;

  spi_chip_down();

  state(State::init);
}
