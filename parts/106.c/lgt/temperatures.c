
::nc::mk::Buffer &
::nc::lgt::to_buffer(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Temperatures & temperatures)
{
  for (unsigned t = 0; t < ::nc::lgt::temperature_pins_COUNT; ++t) {
    for (unsigned g = 0; g < ::nc::lgt::temperature_groups_COUNT; ++g) {
      b << ' '
        << temperatures[t][g];
    }
  }
  return b;
}
