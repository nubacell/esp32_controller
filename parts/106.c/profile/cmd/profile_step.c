
void profile_step(const char * args)
{
  args = skip_whitespace(args);
  if (!*args) {
    LOG("format: step_number"
        " step_ms"
        " sub_step_ms"
        " I_A"
        " PS_V_max"
        " cBat_max_mV"
        " cBat_min_mV"
        " aBat_max_mV"
        " aBat_min_mV"
        " cBat_T_max_C"
        " aBat_T_max_C");
  } else {
    char * endptr = NULL;
    unsigned step = strtoul(args, &endptr, 0);

    if (step >= profile.steps_count_) {
      LOG("ERROR: step=%u >= profile.steps_count=%u", step, profile.steps_count_);
    } else {
      ProfileStep * profileStep = &profileSteps_[step];
      if (endptr && *endptr) {
        profileStep->apply(endptr);
      } else {
        profileStep->dump();
      }
    }
  }
}
