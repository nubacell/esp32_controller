
void stats_disable(const char * args)
{
  (void) args;
  ::nc::loop::stats_.disable();
}
