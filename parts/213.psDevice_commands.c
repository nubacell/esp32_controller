
void ps_id(const char * args)
{
  (void) args;
  psDevice.id();
}

void ps_v(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.v(v);
  }
  psDevice.v();
}

void ps_c(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.c(v);
  }
  psDevice.c();
}

void ps_ovp(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.ovp(v);
  }
  psDevice.ovp();
}

void ps_ocp(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    char * endptr = NULL;
    double v = strtod(args, &endptr);
    if (endptr && *endptr) {
      LOG("ignoring '%s'", endptr);
    }
    psDevice.ocp(v);
  }
  psDevice.ocp();
}

void ps_output(const char * args)
{
  args = skip_whitespace(args);
  if (*args) {
    if (!strcasecmp(args, "on")) {
      psDevice.output_ON();
    } else
    if (!strcasecmp(args, "off")) {
      psDevice.output_OFF();
    } else {
      LOG("output must be ON or OFF");
    }
  }
  psDevice.output();
}
