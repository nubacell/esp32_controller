
double command_args_to_ud(const char * args, const char ** tail)
{
  double v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!*args) {
    v = 0;
  } else {
    v = strtod(args, &endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (endptr) {
    endptr = skip_whitespace(endptr);
  }
  //LOG(". . endptr=%p='%s'", endptr, endptr ? endptr : "<nullptr>");

  if (tail) {
    *tail = endptr;
  } else if (endptr && *endptr) {
    LOG("ignoring '%s'", endptr);
  }

  return v;
}
