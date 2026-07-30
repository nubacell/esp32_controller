
void profile_steps(const char * args)
{
  //LOG("args=%p", args);
  args = skip_whitespace(args);
  //LOG("args=%p", args);
  if (*args) {
    char * endptr = NULL;
    uint32_t new_steps_count = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    LOG("new_steps_count=%u", new_steps_count);
    profile.steps_count(new_steps_count);
  }
  LOG("profile_steps=%u", profile.steps_count_);
}

void profile_steps_dump(const char * args)
{
  (void) args;
  profile.dump_steps();
}

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
      }
      profileStep->dump();
    }
  }
}
