
void Test::change_PS_EL_configs(const ProfileStep * profileStep)
{
  if (PS_V_max != profileStep->PS_V_max) {
    LOG("'PS_V_max' change (%u -> %u)", PS_V_max, profileStep->PS_V_max);
    psDevice.off();
    elDevice.off();
    if (profileStep->I_A > 0) {
      psDevice.v(profileStep->PS_V_max);
      psDevice.c(+ profileStep->I_A);
      elDevice.c(0);
      psDevice.on();
    } else if (profileStep->I_A < 0) {
      psDevice.v(profileStep->PS_V_max);
      psDevice.c(0);
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  } else
  if (sign(I_A) != sign(profileStep->I_A)) {
    LOG("'I_A' direction change (%i -> %i)", I_A, profileStep->I_A);
    if (profileStep->I_A > 0) {
      elDevice.c(0);
      psDevice.c(+ profileStep->I_A);
      psDevice.on();
    } else
    if (profileStep->I_A < 0) {
      psDevice.c(0);
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  } else
  if (I_A != profileStep->I_A) {
    LOG("'I_A' value change (%i -> %i)", I_A, profileStep->I_A);
    if (profileStep->I_A > 0) {
      psDevice.c(+ profileStep->I_A);
      psDevice.on();
    } else
    if (profileStep->I_A < 0) {
      elDevice.c(- profileStep->I_A);
      elDevice.on();
    } else {
      psDevice.c(0);
      elDevice.c(0);
    }
  }
  PS_V_max = profileStep->PS_V_max;
  I_A = profileStep->I_A;
}
