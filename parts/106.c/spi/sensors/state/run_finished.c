
namespace nc {
namespace spi {

void Sensors::on_state_run_finished(uint32_t now_ms)
{
  transfer_end_time_ms_ = now_ms;

  //::nc::spi::sensors::validate();

  run_finished_ = true;
  state(State::idle);
}

} // namespace spi
} // namespace nc
