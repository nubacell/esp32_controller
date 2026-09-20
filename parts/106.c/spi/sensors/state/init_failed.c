
namespace nc {
namespace spi {

// Can be used to implement delay before init retry
void Sensors::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
  state(State::idle);
}

} // namespace spi
} // namespace nc
