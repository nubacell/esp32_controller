
void el_id(const char * args)
{
  (void) args;
  elDevice.id();
}

/*
void el_release_control(const char * args)
{
  (void) args;
  elDevice.release_control();
}
*/

void el_clear_errors(const char * args)
{
  elDevice.clear_errors();
}

void el_function(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "cc")) {
      elDevice.function_CC();
    } else
    if (!strcasecmp(args, "cv")) {
      elDevice.function_CV();
    } else {
      LOG("function must be CC or CV");
    }
  }
  elDevice.function();
}

void el_input(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "on")) {
      elDevice.on();
    } else
    if (!strcasecmp(args, "off")) {
      elDevice.off();
    } else {
      LOG("input must be ON or OFF");
    }
  }
  elDevice.get_input();
}

void el_cc_level(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cc_level(v);
  }
  elDevice.cc_level();
}

void el_cv_level(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cv_level(v);
  }
  elDevice.cv_level();
}

void el_cc_range(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    unsigned v = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cc_range(v);
  }
  elDevice.cc_range();
}

void el_cv_range(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    unsigned v = strtoul(args, &endptr, 0);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    elDevice.cv_range(v);
  }
  elDevice.cv_range();
}
