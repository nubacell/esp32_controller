
void stats_enable(const char * args)
{
  (void) args;
  ::nc::loop::stats_.enable();
}
