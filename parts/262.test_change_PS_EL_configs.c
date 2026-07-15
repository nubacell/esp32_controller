
void Test::change_PS_EL_configs(const ProfileStep * profileStep)
{
  if (ELState::off == profileStep->elconfig_.state_) {
    LOG("deactivatin EL");
    elDevice.set_input_OFF();
  } else if (ELState::CC == profileStep->elconfig_.state_) {
    LOG("changing EL config");
    elDevice.cc_range(profileStep->elconfig_.range_);
    elDevice.cc_level(profileStep->elconfig_.level_);
    elDevice.cc_level();
    elDevice.set_input_ON();
  } else {
    LOG("ERROR: Unexpected EL state");
  }
}
