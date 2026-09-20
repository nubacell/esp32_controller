
namespace nc {
namespace spi {

void Sensors::on_state_transfer_timeout(uint32_t now_ms)
{
  LOG("WARNING: spi sensor %u transfer timeout", sensor_n_);

  // make sure MR pin is inactive
  gpio_set_level(gpio_num_t(SPI_MR_GPIO), SPI_MR_INACTIvE);

  hw.chip_.down();

  state(State::next_sensor);
}

} // namespace spi
} // namespace nc
