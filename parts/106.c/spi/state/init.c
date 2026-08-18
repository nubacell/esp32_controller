
void SPI_Device::on_state_init(uint32_t now_ms)
{
  (void) now_ms;
  state( (spi_chip_init()) ? State::idle : State::init_failed);
}
