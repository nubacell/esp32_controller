
void Test::deactivate_PS_EL()
{
  LOG("deactivating EL");
  psDevice.off();
  elDevice.off();
  PS_V_max = 0;
  I_A = 0;
}
