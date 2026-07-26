
int32_t command_args_to_i32(const char * args, const char ** tail)
{
  unsigned v;
  char * endptr = NULL;

  args = skip_whitespace(args);
  if (!*args) {
    v = 0;
  } else {
    v = strtol(args, &endptr, 0);
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
