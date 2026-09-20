
namespace nc {
namespace spi {

void Sensors::state(State new_state)
{
#if defined nc_Monitor_state_verbose
  const auto old_state_name = to_string(state_);
#endif
  state_ = new_state;
#if defined nc_Monitor_state_verbose
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
#endif
}

} // namespace spi
} // namespace nc
