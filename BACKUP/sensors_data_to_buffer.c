
::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const ::nc::lgt::AnalogSignal & analogSignal)
{
  b.printf(" %4i", analogSignal);
  return b;
}

::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const ::nc::lgt::AnalogSignals & analogSignals)
{
  for (unsigned i = 0; i < ::nc::lgt::ALL_ANALOG_SIGNALS_COUNT; ++i) {
    b << analogSignals.all[i];
  }
  return b;
}

::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const AnalogSignals_u & analogSignals)
{
  b << analogSignals.all;
  return b;
}

::nc::mk::Buffer & operator<<(::nc::mk::Buffer & b,
    const SensorDeviceData_s & sensorDeviceData)
{
  b.printf("  %4u", sensorDeviceData.vcc);
  b << sensorDeviceData.analogSignals;
  return b;
}
