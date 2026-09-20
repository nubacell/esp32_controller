
namespace nc {
namespace spi {

void Sensors::on_state_init(uint32_t now_ms)
{
  (void) now_ms;
  state( (hw.chip_.init()) ? State::idle : State::init_failed);
}

} // namespace spi
} // namespace nc
