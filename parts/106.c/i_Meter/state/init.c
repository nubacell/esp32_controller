
void I_Meter::on_state_init(uint32_t now_ms)
{
  state( (::nc::i2c::adc.init()) ? State::idle : State::init_failed);
}
