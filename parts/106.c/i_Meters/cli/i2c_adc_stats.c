
void i2c_adc_stats(const char * args)
{
  (void) args;
  ::nc::i2c::stats::report();
}
