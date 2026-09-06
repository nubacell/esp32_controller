
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
